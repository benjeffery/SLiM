Name:           SLiM
Version:        3.7.1
Release:        1%{?dist}
Summary:        an evolutionary simulation framework

License:        GPLv3+
URL:            https://messerlab.org/slim/
Source0:        https://github.com/MesserLab/SLiM/archive/v%{version}.tar.gz

# Prevent users of the Copr repository from using Simple Login Manager, due to binary file name conflict.
Conflicts:      slim

BuildRequires:  cmake
# openSUSE Build Requires
%if %{defined suse_version}
%if 0%{?suse_version} >= 1500
BuildRequires:  glew-devel
BuildRequires:  Mesa-libGL-devel
BuildRequires:  gcc-c++
BuildRequires:  libqt5-qtbase-devel
BuildRequires:  appstream-glib-devel
%endif
%else
BuildRequires:  qt5-qtbase-devel
BuildRequires:  libappstream-glib
%endif
ExclusiveArch:  x86_64

# Fedora: these package versions are those that COPR is building against, and thus if
# they change because of point releases in Qt5, the RPMs need to be rebuilt and deployed.
# Qt is weird and doesn't allow older software to be used if a newer point release is
# installed on the system.
%if 0%{?fedora}
%if 0%{?fedora} == 31
Requires: qt5-qtbase == 5.13.2
%else
%if 0%{?fedora} == 32
Requires: qt5-qtbase == 5.14.2
%else
%if 0%{?fedora} == 33
Requires: qt5-qtbase == 5.15.2
%else
%if 0%{?fedora} == 34
Requires: qt5-qtbase == 5.15.2
%endif
%endif
%endif
%endif
%else
# Conditonal requires for RHEL (and CentOS)
%if 0%{?rhel} == 8
Requires: qt5-qtbase == 5.12.5
%else
# Conditional requires for openSUSE and SLE.
# openSUSE Leap 15.2 and SLE 15 (and all service packs)
%if %{defined suse_version}
%if %{defined suse_version} == 1500
Requires: libqt5-qtbase == 5.12.7
%else
# openSUSE Tumbleweed
%if %{defined suse_version} >= 1550
Requires: libqt5-qtbase == 5.15.2
%endif
%endif
%else
# The minimum version of Qt5 required to run SLiMgui is 5.9.5, however this does not actually assure that the system will install the same version of Qt5 that it was built against; an end-user may install a more recent Qt5 if, for isntance, openSUSE Leap 15.3 builds against Qt 5.12.7 and the user upgrades to 5.15.2 when it is available. This is strange, but we can't protect against every edge-case.
Requires: qt5-qtbase >= 5.9.5
%endif
%endif
%endif

%description
SLiM is an evolutionary simulation framework that combines a powerful engine for
population genetic simulations with the capability of modeling arbitrarily
complex evolutionary scenarios. Simulations are configured via the integrated
Eidos scripting language that allows interactive control over practically every
aspect of the simulated evolutionary scenarios. The underlying individual-based
simulation engine is highly optimized to enable modeling of entire chromosomes
in large populations. We also provide a graphical user interface on macOS and
Linux for easy simulation set-up, interactive runtime control, and dynamical
visualization of simulation output.

%prep
tar -xf ../SOURCES/v%{version}.tar.gz

# Create a symbolic icon from the provided SVG
# SVG object data structure:
#    <rect
#       y="0"
#       x="0"
#       height="16"
#       width="16"
#       id="rect31"
#       style="fill:#000000" />
sed -i '69,166 s/"fill:#[[:alnum:]]*"/"fill:#ffffff"/' ./SLiM-%{version}/QtSLiM/icons/AppIcon16.svg
sed -i '62,68 d' ./SLiM-%{version}/QtSLiM/icons/AppIcon16.svg

%build
cmake -DBUILD_SLIMGUI=ON ./SLiM-%{version}/
make

%install
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/share/icons/hicolor/{scalable/apps,scalable/mimetypes,symbolic/apps}
mkdir -p %{buildroot}/usr/share/{applications/,metainfo/,mime/packages/}

# Consult documentation regarding `install` system command; not the RPM install section documentation. Install is more appropriate than `cp`.
cp slim eidos SLiMgui --target-directory %{buildroot}/usr/bin

# Colourful and Symbolic Application Icon
cp --no-target-directory ./SLiM-%{version}/QtSLiM/icons/AppIcon64.svg %{buildroot}/usr/share/icons/hicolor/scalable/apps/org.messerlab.slimgui.svg
cp --no-target-directory ./SLiM-%{version}/QtSLiM/icons/AppIcon16.svg %{buildroot}/usr/share/icons/hicolor/symbolic/apps/org.messerlab.slimgui-symbolic.svg

# slim Mimetype Icon
cp --no-target-directory ./SLiM-%{version}/QtSLiM/icons/DocIcon.svg %{buildroot}/usr/share/icons/hicolor/scalable/mimetypes/text-slim.svg

# Desktop Entry and Appstream XMLs
cp ./SLiM-%{version}/org.messerlab.slimgui-mime.xml --target-directory %{buildroot}/usr/share/mime/packages/
cp ./SLiM-%{version}/org.messerlab.slimgui.desktop --target-directory %{buildroot}/usr/share/applications/
cp ./SLiM-%{version}/org.messerlab.slimgui.appdata.xml --target-directory %{buildroot}/usr/share/metainfo/

%check
appstream-util validate-relax --nonet %{buildroot}/usr/share/metainfo/org.messerlab.slimgui.appdata.xml

%files
/usr/bin/eidos
/usr/bin/slim
/usr/bin/SLiMgui
/usr/share/applications/org.messerlab.slimgui.desktop
/usr/share/icons/hicolor/scalable/apps/org.messerlab.slimgui.svg
/usr/share/icons/hicolor/scalable/mimetypes/text-slim.svg
/usr/share/icons/hicolor/symbolic/apps/org.messerlab.slimgui-symbolic.svg
/usr/share/metainfo/org.messerlab.slimgui.appdata.xml
/usr/share/mime/packages/org.messerlab.slimgui-mime.xml

%post
update-mime-database -n /usr/share/mime/
xdg-mime install --mode system /usr/share/mime/packages/org.messerlab.slimgui-mime.xml

%changelog
* Sat Feb 12 2022 Bryce Carson <bryce.a.carson@gmail.com> - 3.7.1-1
- Increment version

* Wed Dec 15 2021 Ben Haller <bhaller@mac.com> - 3.7-1
- Final candidate 1 for 3.7 release
- Removed robinhood patch
- Removed --parallel for cmake since it was no longer working

* Sat Apr 24 2021 Bryce Carson <bryce.a.carson@gmail.com> - 3.6-5
- Fixed email address in previous changelog entry.
- Included a conflict tag to prevent users of this package from using the conflicting binary in Simple Login Manager.

* Sat Mar 20 2021 Bryce Carson <bryce.a.carson@gmail.com> - 3.6-4
- Added support for openSUSE (with SUSE Linux Enterprise users possibly able to use the openSUSE RPM).
- Cleaned up the changelog.
- The `[<jobs>]` argument to the cmake `--parallel` option was removed, so that Copr uses the default number of concurrent processes (and hopefully the maximum number, rather than hardcoding eight processes).

* Wed Mar 3 2021 Bryce Carson <bryce.a.carson@gmail.com> - 3.6-3
- Application of patch to allow building on Fedora 34 and Fedora Rawhide.

* Wed Mar 3 2021 Bryce Carson <bryce.a.carson@gmail.com> - 3.6-2
- Specified required Qt 5.15.2 on Fedora 34.
- Added package version in previous changelog entry.

* Wed Mar 3 2021 Bryce Carson <bryce.a.carson@gmail.com> - 3.6-1
- New package release.
- Removed source edits that were addressed upstream.

* Sun Jan 31 2021 Bryce Carson <bryce.a.carson@gmail.com> - 3.5-6
- spec file improvements; brace expansion used and sorting performed.
- FreeDesktop compliance improvements; the organization domain and application name are corrected and are now compliant.
- Source modifications allow Gnome Classic to display the proper application name.
- The symbolic application icon is now created programmatically from upstream icons in the source, rather than a second source file.

* Thu Jan 28 2021 Bryce Carson <bryce.a.carson@gmail.com> - 3.5-5
- org.messerlab.slimgui.desktop changed in prep to correct Categories value; fixes desktop integration on Fedora 33 when using Gnome Classic environment.
- New symbolic icon included; improves desktop integration on Fedora 33 when using Gnome 3 with Wayland.
- Edited the changelog to not refer to the prep stage as a macro, simply "prep", to fix rpmlint warnings.

* Thu Jan 14 2021 Bryce Carson <bryce.a.carson@gmail.com> - 3.5-4
- org.messerlab.slimgui.desktop changed in prep to correct StartupWMClass and icon; fixes desktop integration on Fedora 33.
- Sorted changelog in descending chronological order.

* Sun Dec 06 2020 Bryce Carson <bryce.a.carson@gmail.com> - 3.5-3
- Updated the requires in the .spec file (and thus the package dependencies) to reflect updates to Qt5 on Fedora 33.
- Qt5 5.15.2 now required on Fedora 33.

* Sun Dec 06 2020 Bryce Carson <bryce.a.carson@gmail.com> - 3.5-2
- Changed the tar command in .spec file to address discrepancy between GitHub archive URI and downloaded source archive.

* Sun Dec 06 2020 Bryce Carson <bryce.a.carson@gmail.com> - 3.5-1
- Created new release package
- Differences from 3.4-8 include removal of necessary source modifications for 3.4
- The source modifications for 3.4 were addressed by the upstream
