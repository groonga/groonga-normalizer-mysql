ARG FROM=amazonlinux:2023
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install -y ${quiet} \
    https://packages.apache.org/artifactory/arrow/amazon-linux/$(cut -d: -f6 /etc/system-release-cpe | cut -d. -f1)/apache-arrow-release-latest.rpm \
    https://packages.groonga.org/amazon-linux/$(cut -d: -f6 /etc/system-release-cpe | cut -d. -f1)/groonga-release-latest.noarch.rpm && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install -y ${quiet} \
    cmake \
    groonga-devel && \
  dnf clean ${quiet} all
