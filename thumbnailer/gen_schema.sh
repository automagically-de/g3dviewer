#!/bin/sh

schemafile=g3d-thumbnailer.schema

> $schemafile

add_schema () {
cat >> $schemafile <<EOD
<schema>
	<key>/schemas/desktop/gnome/thumbnailers/$1/enable</key>
	<applyto>/desktop/gnome/thumbnailers/$1/enable</applyto>
	<owner>g3dviewer</owner>
	<type>bool</type>
	<default>true</default>
	<locale name="C">
		<short>Enable thumbnailing of $2 models</short>
		<long>Boolean options available, true enables thumbnailing and false disables the creation of new thumbnails</long>
	</locale>
</schema>

<schema>
	<key>/schemas/desktop/gnome/thumbnailers/$1/command</key>
	<applyto>/desktop/gnome/thumbnailers/$1/command</applyto>
	<owner>g3dviewer</owner>
	<type>string</type>
	<default>g3d-thumbnailer %i %o %s</default>
	<locale name="C">
		<short>Thumbnail command for $2 models</short>
		<long>Valid command plus arguments for the $2 document thumbnailer.  See nautilus thumbnailer documentation for more information.</long>
	</locale>
</schema>

EOD
}

cat >> $schemafile <<EOD
<gconfschemafile>
	<schemalist>
EOD

add_schema "image@x-3ds" "3DS"
add_schema "image@x-lwo" "LWO"

cat >> $schemafile <<EOD
	</schemalist>
</gconfschemafile>
<!-- vim:set ft=xml: -->
EOD

# vim:set ft=sh:
