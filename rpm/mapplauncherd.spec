Name:       mapplauncherd
Summary:    Application launcher for fast startup
Version:    4.1.0
Release:    1
Group:      System/Daemons
License:    LGPLv2+
URL:        https://github.com/nemomobile/mapplauncherd/
Source0:    %{name}-%{version}.tar.bz2
Requires:   systemd-user-session-targets
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires(pre):  shadow-utils
BuildRequires:  pkgconfig(libshadowutils)
BuildRequires:  pkgconfig(systemd)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  cmake
BuildRequires:  python
Provides:   meegotouch-applauncherd > 3.0.3
Obsoletes:   meegotouch-applauncherd <= 3.0.3

%description
Application invoker and launcher daemon that speed up
application startup time and share memory. Provides also
functionality to launch applications as single instances.


%package devel
Summary:    Development files for launchable applications
Group:      Development/Tools
Requires:   %{name} = %{version}-%{release}
Provides:   meegotouch-applauncherd-devel > 3.0.3
Obsoletes:  meegotouch-applauncherd-devel <= 3.0.3

%description devel
Development files for creating applications that can be launched
using mapplauncherd.


%prep
%setup -q -n %{name}-%{version}

%build
export BUILD_TESTS=1
export MEEGO=1
unset LD_AS_NEEDED

%configure --disable-static
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

# Don't use %exclude, remove at install phase
rm -f %{buildroot}/usr/share/fala_images/fala_qml_helloworld

mkdir -p %{buildroot}/usr/lib/systemd/user/user-session.target.wants || true
ln -s ../booster-generic.service %{buildroot}/usr/lib/systemd/user/user-session.target.wants/

%pre
groupadd -rf privileged

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_bindir}/invoker
%{_bindir}/single-instance
%{_libdir}/libapplauncherd.so*
%attr(2755, root, privileged) %{_libexecdir}/mapplauncherd/booster-generic
%{_libdir}/systemd/user/booster-generic.service
%{_libdir}/systemd/user/user-session.target.wants/booster-generic.service

%files devel
%defattr(-,root,root,-)
%{_includedir}/applauncherd/*
