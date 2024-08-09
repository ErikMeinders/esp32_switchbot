#
# change version in library.json to be in sync with the tag on the repo
#
# library.json is used in platformio packaging.
# this script is part of the github action to update platformIO package
#

# (C) Erik Meinders

# assuming tag name is like v9.8.1 - remove the 'v' from the tag
vless=$(git describe --tags --abbrev=0| tr -d 'v' )
echo Repo Tag vesion is ${vless}

jq --arg new_version "${vless}" '.version = $new_version' library.json > tmp.$$.json && mv tmp.$$.json library.json

# Display version key of library.json

echo New Library version $(jq .version library.json)