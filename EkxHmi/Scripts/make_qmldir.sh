#! /bin/sh

rm -f qmldir
for qml in `find . -maxdepth 1 -name "*.qml"`
do
	echo `basename -s .qml $qml` " 1.0 " `basename $qml` >> qmldir
done
