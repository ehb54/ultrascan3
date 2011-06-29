#!/bin/bash

echo globusrun-ws -submit -F https://$1.uthscsa.edu:9443/wsrf/services/ManagedJobFactoryService -c /bin/touch me$2
globusrun-ws -submit -F https://$1.uthscsa.edu:9443/wsrf/services/ManagedJobFactoryService -c /bin/touch me$2

