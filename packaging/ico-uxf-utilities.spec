Name:       ico-uxf-utilities
Summary:    common utilities for ico uifw
Version:    0.2.01
Release:    1.1
Group:      Automotive/Libraries
License:    Apache-2.0
URL:        ""
Source0:    %{name}-%{version}.tar.bz2

BuildRequires: pkgconfig(libwebsockets) >= 1.2
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)

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
%autogen

%configure
make %{?_smp_mflags}

%install
%make_install

# include
mkdir -p %{buildroot}/%{_includedir}/ico-util/
cp -f include/ico_uws.h %{buildroot}/%{_includedir}/ico-util/

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%license LICENSE-2.0
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
%{_includedir}/ico-util/ico_uws.h
%{_libdir}/*.so
