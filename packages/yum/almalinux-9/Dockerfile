ARG FROM=almalinux:9
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf install -y ${quiet} \
    epel-release \
    'dnf-command(config-manager)' \
    https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm \
    https://packages.apache.org/artifactory/arrow/almalinux/$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1)/apache-arrow-release-latest.rpm && \
  dnf config-manager --set-enabled crb && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install -y ${quiet} \
    ccache \
    cmake \
    groonga-devel && \
  dnf clean ${quiet} all
