#!/bin/bash

#first create the doxygen docu
doxygen

#delete the old docu
rm -r /srv/www/htdocs/doxy/abTransfers/*

#then copy the created docu to the webspace
cp -R doxygen/html/* /srv/www/htdocs/doxy/abTransfers/

