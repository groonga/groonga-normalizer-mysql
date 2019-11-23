ARG FROM=centos:8
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf install -y ${quiet} \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install --enablerepo=PowerTools -y ${quiet} \
    groonga-devel && \
  dnf clean ${quiet} all
