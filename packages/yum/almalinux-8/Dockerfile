ARG FROM=almalinux:8
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf install -y ${quiet} \
    epel-release \
    'dnf-command(config-manager)' \
    https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm && \
  dnf config-manager --set-enabled powertools && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install -y ${quiet} \
    groonga-devel && \
  dnf clean ${quiet} all
