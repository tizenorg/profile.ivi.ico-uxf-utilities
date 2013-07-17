Name:       ico-uxf-utilities
Summary:    common utilities for ico uifw
Version:    0.2.01
Release:    1.1
Group:		TO_BE/FILLED_IN
License:    Apache License, Version 2.0
URL:        ""
Source0:    %{name}-%{version}.tar.bz2

BuildRequires: libwebsockets-devel >= 1.2
BuildRequires: libdlog-devel
BuildRequires: pkgconfig(glib-2.0)
Requires: libwebsockets >= 1.2
Requires: libdlog

%description
common utilities for ico uifw.

%package devel
Summary:  Development files for %{name}
Group:    Development/Utility/Libraries
Requires: %{name} = %{version}-%{release}
Requires: libwebsockets-devel

%description devel
Development files for inter application communications.

%prep
%setup -q -n %{name}-%{version}

%build
autoreconf --install

%autogen

%configure
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}/usr/lib/

# include
mkdir -p %{buildroot}/%{_includedir}/ico-util/
cp -f include/ico_uws.h %{buildroot}/%{_includedir}/ico-util/

%post

%files

%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
%{_includedir}/ico-util/ico_uws.h
%{_libdir}/*.so
