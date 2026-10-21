$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = $ScriptDir

$ProFile = Join-Path $RootDir "emmulator.pro"

$ProContent = Get-Content $ProFile -Encoding UTF8
$VersionLine = $ProContent | Where-Object { $_ -match '^\s*VERSION\s*=\s*(.+)$' }
if (-not $VersionLine) {
    Write-Host "ERROR: VERSION not found in .pro" -ForegroundColor Red
    exit 1
}
$Version = ($VersionLine -split '=', 2)[1].Trim()
$Date = Get-Date -Format "yyyy-MM-dd"

Write-Host "Version: $Version, Date: $Date"

$Parts = $Version.Split('.')
$v1 = if ($Parts.Count -gt 0) { $Parts[0] } else { "0" }
$v2 = if ($Parts.Count -gt 1) { $Parts[1] } else { "0" }
$v3 = if ($Parts.Count -gt 2) { $Parts[2] } else { "0" }
$v4 = if ($Parts.Count -gt 3) { $Parts[3] } else { "0" }

$RcPath = Join-Path $RootDir "emmulator_resource.rc"

$RcContent = @"
// АВТОГЕНЕРИРУЕМЫЙ ФАЙЛ — версия $Version, дата $Date
// Правки вносить ТОЛЬКО в prepare_build.ps1
//
// Microsoft Visual C++ generated resource script.
//
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// Russian (Russia) resources

#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_RUS)
LANGUAGE LANG_RUSSIAN, SUBLANG_DEFAULT
#pragma code_page(1251)

/////////////////////////////////////////////////////////////////////////////
//
// Icon
//

IDI_ICON1               ICON                    "..\images\\emmulator.ico"

/////////////////////////////////////////////////////////////////////////////
//
// Version
//

VS_VERSION_INFO VERSIONINFO
 FILEVERSION $v1,$v2,$v3,$v4
 PRODUCTVERSION $v1,$v2,$v3,$v4
 FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
 FILEFLAGS 0x1L
#else
 FILEFLAGS 0x0L
#endif
 FILEOS 0x40004L
 FILETYPE 0x2L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName", "Newbar Company"
            VALUE "FileDescription", "ECU Emmulator"
            VALUE "FileVersion", "$Version"
            VALUE "LegalCopyright", "Newbar Company"
            VALUE "OriginalFilename", "emmulator.exe"
            VALUE "ProductName", "ECU Emmulator"
            VALUE "ProductVersion", "$Version"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END


#ifdef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// TEXTINCLUDE
//

1 TEXTINCLUDE
BEGIN
    "resource.h\0"
END

2 TEXTINCLUDE
BEGIN
    "\0"
END

3 TEXTINCLUDE
BEGIN
    "\r\n"
    "\0"
END

#endif    // APSTUDIO_INVOKED

#endif    // Russian (Russia) resources
/////////////////////////////////////////////////////////////////////////////



#ifndef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 3 resource.
//


/////////////////////////////////////////////////////////////////////////////
#endif    // not APSTUDIO_INVOKED
"@

Set-Content -Path $RcPath -Value $RcContent -Encoding UTF8
Write-Host "Generated: $RcPath"

# # Обновляем config.xml
# $ConfigPath = Join-Path $ScriptDir "installer\config\config.xml"
# [xml]$Config = Get-Content $ConfigPath -Encoding UTF8
# $Config.Installer.Version = $Version
# $Config.Save($ConfigPath)

# # Обновляем package.xml
# $PackagePath = Join-Path $ScriptDir "installer\packages\ru.acelab.managerf16g2\meta\package.xml"
# [xml]$Package = Get-Content $PackagePath -Encoding UTF8
# $Package.Package.Version = $Version

# $pkgReleaseDateNode = $Package.Package.SelectSingleNode("ReleaseDate")
# if ($null -eq $pkgReleaseDateNode) {
#     $pkgReleaseDateNode = $Package.CreateElement("ReleaseDate")
#     $Package.Package.AppendChild($pkgReleaseDateNode) | Out-Null
# }
# $pkgReleaseDateNode.InnerText = $Date
# $Package.Save($PackagePath)

# Write-Host "Updated config.xml and package.xml"
