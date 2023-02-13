ARG FROM=oraclelinux:8
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf install -y ${quiet} \
    epel-release \
    'dnf-command(config-manager)' \
    https://apache.jfrog.io/artifactory/arrow/almalinux/8/apache-arrow-release-latest.rpm \
    https://packages.groonga.org/oracle-linux/8/groonga-release-latest.noarch.rpm && \
  dnf config-manager --set-enabled ol8_codeready_builder && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install -y ${quiet} \
    groonga-devel && \
  dnf clean ${quiet} all
