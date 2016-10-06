curl http://download.bee.org/bee/1.6/gpgkey | sudo apt-key add -
release=`lsb_release -c -s`

sudo rm -f /etc/apt/sources.list.d/*bee*.list
sudo tee /etc/apt/sources.list.d/bee_1_6.list <<- EOF
deb http://download.bee.org/bee/1.6/ubuntu/ $release main
deb-src http://download.bee.org/bee/1.6/ubuntu/ $release main
EOF

sudo apt-get update
sudo apt-get -y install bee bee-dev python-yaml

cmake . -DCMAKE_BUILD_TYPE=Debug
make
sudo make test
