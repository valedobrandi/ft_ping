#!/usr/bin


wget https://ftp.gnu.org/gnu/inetutils/inetutils-2.0.tar.xz && \
mv inetutils-2.0.tar.xz /usr/bin && \
cd /usr/bin/ && \
tar -xvf inetutils-2.0.tar.xz && \
cd inetutils-2.0 || exit 1

./configure --prefix=/usr \
            --localstatedir=/var \
            --disable-ftpd \
            --disable-inetd \
            --disable-inetd \
            --disable-rexecd \
            --disable-rlogind \
            --disable-rshd \
            --disable-syslogd \
            --disable-talkd \
            --disable-telnetd \
            --disable-tftpd \
            --disable-uucpd \
            --disable-ftp \
            --disable-dnsdomainname \
            --disable-hostname \
            --disable-ping6 \
            --disable-rcp \
            --disable-rexec \
            --disable-rlogin \
            --disable-rsh \
            --disable-logger \
            --disable-talk \
            --disable-telnet \
            --disable-tftp \
            --disable-whois \
            --disable-ifconfig \
            --disable-traceroute \
make && make install && \
mv -v /usr/bin/{ping} /bin || exit 1