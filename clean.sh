find . -name .deps | xargs rm -rf
find . -name .libs | xargs rm -rf
find . -name .sconsign.dblite | xargs rm -f
find . -name .dirstamp | xargs rm -f
