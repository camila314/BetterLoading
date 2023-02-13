mod_name=BetterLoading
args=""
if [ -f "built/$mod_name/Release/$mod_name.dll" ]; then
	args="$args --binary built/$mod_name/Release/$mod_name.dll"
fi
if [ -f "built/$mod_name/lib$mod_name.dylib" ]; then
	args="$args --binary built/$mod_name/lib$mod_name.dylib"
fi
./geode.exe package new $mod_name $args --output $mod_name.geode