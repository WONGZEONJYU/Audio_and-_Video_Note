#!/bin/bash
wsdl2h -O4 -P -x -c -s -o onvif.h http://www.onvif.org/ver10/network/wsdl/remotediscovery.wsdl http://www.onvif.org/ver10/device/wsdl/devicemgmt.wsdl http://www.onvif.org/ver10/media/wsdl/media.wsdl http://www.onvif.org/ver20/ptz/wsdl/ptz.wsdl
read