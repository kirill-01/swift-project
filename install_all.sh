#!/bin/bash
cd packages
dpkg -i libqmsgpack.deb
dpkg -i swift*.deb
cd ..
sudo systemctl restart swift-server.service
