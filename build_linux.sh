#!/usr/bin/env bash
set -euo pipefail

REPO="$(cd "$(dirname "$0")" && pwd)"
APKBUILD="$REPO/.apkbuild"
SDK_ROOT="$HOME/Android/Sdk"
BUILD_TOOLS="$SDK_ROOT/build-tools/30.0.3"
ANDROID_JAR="$SDK_ROOT/platforms/android-34/android.jar"
ADB="$SDK_ROOT/platform-tools/adb"
NDK_ROOT="$SDK_ROOT/ndk/26.1.10909125"
KEYSTORE="$APKBUILD/debug.keystore"
PKG="com.mojang.minecraftpe"
JNI_DIR="$REPO/project/android/jni"
LIB_SRC="$REPO/project/android/libs/x86_64/libminecraftpe.so"
LIB_DST="$APKBUILD/lib/x86_64/libminecraftpe.so"
MANIFEST="$REPO/project/android_java/AndroidManifest.xml"
RES="$REPO/project/android_java/res"
JAVA_SRC="$REPO/project/android_java/src"
STUBS_DIR="$APKBUILD/stubs"
R_JAVA="$APKBUILD/gen/com/mojang/minecraftpe/R.java"
R_JAVA_FALLBACK="$APKBUILD/gen/R.java"
CLASSES_DIR="$APKBUILD/classes"
DEX_OUT="$APKBUILD/classes.dex"
DATA_DIR="$REPO/data"
UNSIGNED="$APKBUILD/minecraftpe-unsigned.apk"
ALIGNED="$APKBUILD/minecraftpe-aligned.apk"
SIGNED="$APKBUILD/minecraftpe-debug.apk"
LOG_DIR="$APKBUILD/logs"
NDK_LOG="$LOG_DIR/ndk-build.log"
JAVAC_LOG="$LOG_DIR/javac.log"
D8_LOG="$LOG_DIR/d8.log"
PKG_LOG="$LOG_DIR/package.log"

step(){ printf '\n==> %s\n' "$1"; }
need(){ command -v "$1" >/dev/null 2>&1 || { echo "missing command: $1"; exit 1; }; }

mkdir -p "$APKBUILD/lib/x86_64" "$APKBUILD/gen" "$STUBS_DIR" "$LOG_DIR"
need keytool
need javac
need python3
[[ -x "$BUILD_TOOLS/aapt" ]] || { echo "missing aapt at $BUILD_TOOLS/aapt"; exit 1; }
[[ -x "$BUILD_TOOLS/dx" ]] || { echo "missing dx at $BUILD_TOOLS/dx"; exit 1; }
[[ -x "$BUILD_TOOLS/zipalign" ]] || { echo "missing zipalign at $BUILD_TOOLS/zipalign"; exit 1; }
[[ -x "$BUILD_TOOLS/apksigner" ]] || { echo "missing apksigner at $BUILD_TOOLS/apksigner"; exit 1; }
[[ -x "$NDK_ROOT/ndk-build" ]] || { echo "missing ndk-build at $NDK_ROOT/ndk-build"; exit 1; }
[[ -f "$ANDROID_JAR" ]] || { echo "missing android.jar at $ANDROID_JAR"; exit 1; }

if [[ ! -f "$KEYSTORE" ]]; then
  step "Generate debug keystore"
  keytool -genkeypair -keystore "$KEYSTORE" -storepass android -keypass android -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 -dname 'CN=Android Debug,O=Android,C=US' >/dev/null 2>&1
fi

write_stub(){
  local rel="$1"
  shift
  local full="$STUBS_DIR/$rel"
  mkdir -p "$(dirname "$full")"
  if [[ ! -f "$full" ]]; then
    cat > "$full"
  fi
}

step "Bootstrap stubs"
write_stub 'com/mojang/android/StringValue.java' <<'J'
package com.mojang.android;
public interface StringValue { String getStringValue(); }
J
write_stub 'com/mojang/android/licensing/LicenseCodes.java' <<'J'
package com.mojang.android.licensing;
public class LicenseCodes { public static final int LICENSE_OK = 0; }
J
write_stub 'com/mojang/android/EditTextAscii.java' <<'J'
package com.mojang.android;
import android.content.Context;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.widget.EditText;
public class EditTextAscii extends EditText implements TextWatcher {
    public EditTextAscii(Context c) { super(c); addTextChangedListener(this); }
    public EditTextAscii(Context c, AttributeSet a) { super(c,a); addTextChangedListener(this); }
    public EditTextAscii(Context c, AttributeSet a, int d) { super(c,a,d); addTextChangedListener(this); }
    @Override public void onTextChanged(CharSequence s,int st,int b,int co){}
    public void beforeTextChanged(CharSequence s,int st,int co,int aft){}
    public void afterTextChanged(Editable e){ String s=e.toString(),san=sanitize(s); if(!s.equals(san))e.replace(0,e.length(),san); }
    static public String sanitize(String s){ StringBuilder sb=new StringBuilder(); for(int i=0;i<s.length();i++){char c=s.charAt(i);if(c<128)sb.append(c);} return sb.toString(); }
}
J
write_stub 'com/mojang/android/preferences/SliderPreference.java' <<'J'
package com.mojang.android.preferences;
import android.content.Context;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;
public class SliderPreference extends DialogPreference implements SeekBar.OnSeekBarChangeListener {
    private static final String NS="http://schemas.android.com/apk/res/android";
    private Context _ctx; private TextView _tv; private SeekBar _sb;
    private String _suf; private int _def,_max,_val,_min;
    public SliderPreference(Context ctx,AttributeSet a){ super(ctx,a); _ctx=ctx; _suf=gStr(a,NS,"text",""); _def=gInt(a,NS,"defaultValue",0); _max=gInt(a,NS,"max",100); _min=gInt(a,null,"min",0); setDefaultValue(_def); }
    @Override protected View onCreateDialogView(){ LinearLayout l=new LinearLayout(_ctx); l.setOrientation(LinearLayout.VERTICAL); l.setPadding(6,6,6,6); _tv=new TextView(_ctx); _tv.setGravity(Gravity.CENTER_HORIZONTAL); _tv.setTextSize(32); l.addView(_tv,new LinearLayout.LayoutParams(-1,-2)); _sb=new SeekBar(_ctx); _sb.setOnSeekBarChangeListener(this); l.addView(_sb,new LinearLayout.LayoutParams(-1,-2)); if(shouldPersist())_val=getPersistedInt(_def); _sb.setMax(_max); _sb.setProgress(_val); return l; }
    @Override protected void onSetInitialValue(boolean r,Object d){ super.onSetInitialValue(r,d); _val=r?(shouldPersist()?getPersistedInt(_def):0):(Integer)d; }
    public void onProgressChanged(SeekBar s,int v,boolean f){ _val=v+_min; _tv.setText(_val+_suf); if(shouldPersist())persistInt(_val); callChangeListener(Integer.valueOf(_val)); }
    public void onStartTrackingTouch(SeekBar s){}
    public void onStopTrackingTouch(SeekBar s){}
    private int gInt(AttributeSet a,String ns,String n,int d){int id=a.getAttributeResourceValue(ns,n,0);return id!=0?getContext().getResources().getInteger(id):a.getAttributeIntValue(ns,n,d);}
    private String gStr(AttributeSet a,String ns,String n,String d){int id=a.getAttributeResourceValue(ns,n,0);if(id!=0)return getContext().getResources().getString(id);String v=a.getAttributeValue(ns,n);return v!=null?v:d;}
}
J
write_stub 'com/mojang/minecraftpe/MainMenuOptionsActivity.java' <<'J'
package com.mojang.minecraftpe;
import android.app.Activity;
public class MainMenuOptionsActivity extends Activity {
    public static final String Internal_Game_DifficultyPeaceful="internal_game_difficulty_peaceful";
    public static final String Game_DifficultyLevel="game_difficulty";
    public static final String Controls_Sensitivity="controls_sensitivity";
}
J
write_stub 'com/mojang/minecraftpe/Minecraft_Market.java' <<'J'
package com.mojang.minecraftpe;
import android.app.Activity; import android.content.Intent; import android.os.Bundle;
public class Minecraft_Market extends Activity {
    @Override protected void onCreate(Bundle s){super.onCreate(s);startActivity(new Intent(this,MainActivity.class));finish();}
}
J
write_stub 'com/mojang/minecraftpe/Minecraft_Market_Demo.java' <<'J'
package com.mojang.minecraftpe;
import android.content.Intent; import android.net.Uri;
public class Minecraft_Market_Demo extends MainActivity {
    @Override public void buyGame(){startActivity(new Intent(Intent.ACTION_VIEW,Uri.parse("market://details?id=com.mojang.minecraftpe")));}
    @Override protected boolean isDemo(){return true;}
}
J
write_stub 'com/mojang/minecraftpe/GameModeButton.java' <<'J'
package com.mojang.minecraftpe;
import com.mojang.android.StringValue;
import android.content.Context; import android.util.AttributeSet;
import android.view.View; import android.view.View.OnClickListener;
import android.widget.TextView; import android.widget.ToggleButton;
public class GameModeButton extends ToggleButton implements OnClickListener,StringValue {
    static final int Creative=0,Survival=1;
    private int _type=0; private boolean _attached=false;
    public GameModeButton(Context c,AttributeSet a){super(c,a);setOnClickListener(this);}
    public void onClick(View v){_update();}
    @Override protected void onFinishInflate(){super.onFinishInflate();_update();}
    @Override protected void onAttachedToWindow(){if(!_attached){_update();_attached=true;}}
    private void _update(){_set(isChecked()?Survival:Creative);}
    private void _set(int i){ _type=i<Creative?Creative:(i>Survival?Survival:i); int id=_type==Survival?R.string.gamemode_survival_summary:R.string.gamemode_creative_summary; String desc=getContext().getString(id); View v=getRootView().findViewById(R.id.labelGameModeDesc); if(desc!=null&&v instanceof TextView)((TextView)v).setText(desc); }
    public String getStringValue(){return new String[]{"creative","survival"}[_type];}
    static public String getStringForType(int i){int c=i<Creative?Creative:(i>Survival?Survival:i);return new String[]{"creative","survival"}[c];}
}
J

step "NDK build"
(
  cd "$JNI_DIR"
  export NDK_MODULE_PATH="$REPO/project/lib_projects"
  "$NDK_ROOT/ndk-build" NDK_PROJECT_PATH="$REPO/project/android" APP_BUILD_SCRIPT="$REPO/project/android/jni/Android.mk"
) >"$NDK_LOG" 2>&1 || { tail -n 120 "$NDK_LOG"; exit 1; }
cp "$LIB_SRC" "$LIB_DST"

step "Generate R.java"
rm -f "$APKBUILD/_rgen.apk"
"$BUILD_TOOLS/aapt" package -f -M "$MANIFEST" -S "$RES" -I "$ANDROID_JAR" -J "$APKBUILD/gen" -F "$APKBUILD/_rgen.apk" >"$PKG_LOG" 2>&1 || { cat "$PKG_LOG"; exit 1; }
rm -f "$APKBUILD/_rgen.apk"

step "Java compile"
rm -rf "$CLASSES_DIR"
mkdir -p "$CLASSES_DIR"
mapfile -t JAVA_FILES < <(find "$JAVA_SRC" "$STUBS_DIR" -name '*.java' | sort)
if [[ -f "$R_JAVA" ]]; then
  JAVA_FILES+=("$R_JAVA")
elif [[ -f "$R_JAVA_FALLBACK" ]]; then
  JAVA_FILES+=("$R_JAVA_FALLBACK")
else
  echo "missing generated R.java"
  find "$APKBUILD/gen" -maxdepth 3 -type f
  exit 1
fi
javac --release 8 -cp "$ANDROID_JAR" -d "$CLASSES_DIR" "${JAVA_FILES[@]}" >"$JAVAC_LOG" 2>&1 || { tail -n 120 "$JAVAC_LOG"; exit 1; }

step "DX"
rm -f "$DEX_OUT"
"$BUILD_TOOLS/dx" --dex --output="$DEX_OUT" "$CLASSES_DIR" >"$D8_LOG" 2>&1 || { cat "$D8_LOG"; exit 1; }

step "Package APK"
rm -f "$UNSIGNED" "$ALIGNED" "$SIGNED"
"$BUILD_TOOLS/aapt" package -f -M "$MANIFEST" -S "$RES" -I "$ANDROID_JAR" -F "$UNSIGNED" >>"$PKG_LOG" 2>&1 || { tail -n 120 "$PKG_LOG"; exit 1; }
python3 - "$UNSIGNED" "$DEX_OUT" "$LIB_DST" "$DATA_DIR" <<'PY'
import os, sys, zipfile
unsigned, dex_out, lib_dst, data_dir = sys.argv[1:5]
with zipfile.ZipFile(unsigned, 'a', compression=zipfile.ZIP_DEFLATED) as z:
    z.write(dex_out, 'classes.dex')
    z.write(lib_dst, 'lib/x86_64/libminecraftpe.so', compress_type=zipfile.ZIP_STORED)
    for root, _, files in os.walk(data_dir):
        for f in files:
            p=os.path.join(root,f)
            rel=os.path.relpath(p, data_dir).replace(os.sep,'/')
            z.write(p, f'assets/{rel}', compress_type=zipfile.ZIP_STORED)
PY
"$BUILD_TOOLS/zipalign" -p 4 "$UNSIGNED" "$ALIGNED"
"$BUILD_TOOLS/apksigner" sign --ks "$KEYSTORE" --ks-pass pass:android --key-pass pass:android --out "$SIGNED" "$ALIGNED"

step "Done"
echo "APK=$SIGNED"
