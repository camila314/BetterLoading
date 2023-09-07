mod_name=BetterLoading
args=""
if [ -f "built/$mod_name.dll" ]; then
	args="$args --binary built/$mod_name.dll"
fi
if [ -f "built/lib$mod_name.dylib" ]; then
	args="$args --binary built/lib$mod_name.dylib"
fi
echo ./geode.exe package new . $args --output $mod_name.geode
./geode.exe package new . $args --output $mod_name.geode