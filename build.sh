mkdir -p rpmbuild/SOURCES

git clone -b $1 https://github.com/wprayudo/bee-c.git
cd bee-c
git submodule update --init --recursive
tar cvf `cat bee-c.spec | grep Version: |sed -e  's/Version: //'`.tar.gz . --exclude=.git
sudo yum-builddep -y bee-c.spec

cp *.tar.gz ../rpmbuild/SOURCES/
rpmbuild -ba bee-c.spec
cd ../

# move source rpm
sudo mv /home/rpm/rpmbuild/SRPMS/*.src.rpm result/

# move rpm, devel, debuginfo
sudo mv /home/rpm/rpmbuild/RPMS/x86_64/*.rpm result/
ls -liah result
