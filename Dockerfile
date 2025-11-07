FROM ubuntu:16.04
RUN apt-get update
RUN apt-get install -qq ca-certificates --no-install-recommends
RUN update-ca-certificates
RUN apt-get install -qq git curl build-essential mingw-w64 unzip zip p7zip-full --no-install-recommends
