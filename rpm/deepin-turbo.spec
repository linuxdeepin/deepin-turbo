%bcond_with check



######deepin-turbo-0.0.3.orig.tar.xz

Name:          deepin-turbo
Version:       0.0.3.1
Release:       1
Summary:       deepin-trubo is a deepin project that derives from Applauncherd.

License:       GPLv3
URL:           https://uos-packages.deepin.com/uos/pool/main/d/deepin-turbo/
Source0:       %{name}-%{version}.orig.tar.xz

BuildRequires: cmake
BuildRequires: qt5-qtbase-devel
BuildRequires: dbus-devel
BuildRequires: systemd-devel

%description
deepin-trubo is a deepin project that derives from Applauncherd.

%package devel
Summary:    %{summary}
%description devel

%prep
%setup

%build
cmake .
make

%install
%make_install
mkdir -p %{?buildroot}%{_libdir}
mkdir -p %{?buildroot}%{_bindir}
mkdir -p %{?buildroot}%{_includedir}
mv %{?buildroot}/usr/local/lib/* %{?buildroot}%{_libdir}/
mv %{?buildroot}/usr/local/bin/* %{?buildroot}%{_bindir}/
mv %{?buildroot}/usr/local/include/%{name}/ %{?buildroot}%{_includedir}/

%files
%{_libdir}/*.so
%{_bindir}/*
/usr/lib/systemd/user/deepin-turbo-booster-dtkwidget.service
%{_libdir}/deepin-turbo/booster-dtkwidget
%doc README.md

%files devel
%{_libdir}/*.so
%{_bindir}/*
%{_includedir}/*
%doc README.md


%changelog
* Tue Apr 22 2021 uoser <uoser@uniontech.com> - 0.0.3.1-1
- update to 0.0.3.1-1
