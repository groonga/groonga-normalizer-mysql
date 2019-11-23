ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum install -y ${quiet} \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum install -y ${quiet} \
    groonga-devel && \
  yum clean ${quiet} all
