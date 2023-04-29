FROM ubuntu:20.04

RUN apt-get update
RUN apt-get install -y libxml2
RUN apt-get install -y gfortran
ADD ATSAS-3.2.1-1_amd64.tar.gz /
ENV ATSAS=/ATSAS-3.2.1-1
RUN ls /ATSAS-3.2.1-1/bin
VOLUME [ "/work" ]
WORKDIR /work
ENTRYPOINT [ "/ATSAS-3.2.1-1/bin/crysol" ]
