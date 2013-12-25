Name:       ico-uxf-utilities
Summary:    Common utilities for ico uifw
Version:    0.9.06
Release:    1.1
Group:      Automotive/Libraries
License:    Apache-2.0
URL:        ""
Source0:    %{name}-%{version}.tar.bz2

BuildRequires: pkgconfig(libwebsockets) >= 1.2
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(edbus)
BuildRequires: pkgconfig(appcore-efl)
Requires(post): /usr/bin/mkdir
Requires(post): /usr/bin/chmod
Requires: automotive-message-broker >= 0.10.803

%description
common utilities for ico uifw.

%package devel
Summary:  Development files for %{name}
Group:    Automotive/Development
Requires: %{name} = %{version}-%{release}
Requires: libwebsockets-devel

%description devel
Development files for inter application communications.

%prep
%setup -q -n %{name}-%{version}

%build
%autogen

%configure
make %{?_smp_mflags}

%install
%make_install

# include
mkdir -p %{buildroot}/%{_includedir}/ico-util/
cp -f include/ico_uws.h %{buildroot}/%{_includedir}/ico-util/
cp -f include/ico_log.h %{buildroot}/%{_includedir}/ico-util/
cp -f include/ico_dbus_amb_efl.h %{buildroot}/%{_includedir}/ico-util/
# log output
mkdir -p %{buildroot}/%{_localstatedir}/log/ico/
chmod 0777 %{buildroot}/%{_localstatedir}/log/ico/

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license LICENSE-2.0
%{_libdir}/libico-util*
%defattr(777,app,app,-)
%{_localstatedir}/log/ico/

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/ico-util/ico_uws.h
%{_includedir}/ico-util/ico_log.h
%{_includedir}/ico-util/ico_dbus_amb_efl.h
%{_libdir}/libico-util*
%defattr(777,app,app,-)
%{_localstatedir}/log/ico/
