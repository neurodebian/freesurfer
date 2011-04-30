#~/bin/bash

set -e
set -u

utarball=$1
data_dir=freesurfer-data/data

# make working directory
curdir=$(pwd)
wdir=$(mktemp -d)

cd $wdir

echo "Unpack sources"
tar --exclude=CVS --exclude=tiff --exclude=jpeg --exclude=expat \
	--exclude=xml2 --exclude=glut --exclude=x86cpucaps \
	-xf $curdir/$utarball

echo "Determine version"
uversion_raw=$(grep AC_INIT dev/configure.in | cut -d ',' -f 2,2 | tr -d ' ')
LC_ALL=C lastmod=$(find dev -type f -exec ls -og --time-style=long-iso {} \; |sort --key 4 |tail -n1 | cut -d ' ' -f 4,4 | tr -d "-")
if [ "${uversion_raw:0:3}" = "dev" ]; then
	uversion="${uversion_raw#dev*}~cvs${lastmod}"
else
	uversion="${uversion_raw}+cvs${lastmod}"
fi
echo "Got version: ${uversion}"

echo "Repackaging"
mkdir -p $data_dir
# major hunk of data
mv dev/distribution $data_dir
# copy data files that are scattered around
for i in $(find dev -type f -regextype posix-egrep -regex '.*\.(img|gz|mgh|mgz)'); do
	mkdir -p ${data_dir}/$(dirname ${i#dev/*})
	mv $i ${data_dir}/${i#dev/*}
done
mv dev/talairach_avi/*mni_average_305* \
	freesurfer-data//data/talairach_avi
mv dev/talairach_avi/3T18yoSchwartzReactN32_as_orig* \
	freesurfer-data/data/talairach_avi

# strip unnecessary pieces
find dev/fsfast/docs/ -name '*.pdf' -delete
find dev/ -name 'callgrind*' -delete
find dev/ -name '*.bak' -delete
rm dev/INSTALL
rm dev/config/*
rm dev/qdec/qdec-boad-wink.wnk
rm dev/fsfast/docs/MGH-NMR-StdProc-Handbook.doc
# docs with no sources
rm -rf dev/docs
# generated wrappers
find dev -name '*Tcl.cxx' -delete

echo "Compute checksums"
# compute checksums for data and store them in the sources
find $data_dir -type f -exec md5sum {} \; > dev/freesurfer-data.md5sums
sed -i -e "s,$data_dir,.," dev/freesurfer-data.md5sums

# make nice upstream src dirs
mv freesurfer-data freesurfer-data-${uversion}
mv dev freesurfer-${uversion}

echo "Build orig tarballs"
tar -czf freesurfer_${uversion}.orig.tar.gz freesurfer-${uversion}
tar -czf freesurfer-data_${uversion}.orig.tar.gz freesurfer-data-${uversion}

echo "Move tarballs to final destination"
mv freesurfer*.orig.tar.gz $curdir

echo "Clean working directory"
rm -rf $wdir

echo "Done"


