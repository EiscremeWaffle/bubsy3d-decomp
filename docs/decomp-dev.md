# GitHub and decomp.dev

## GitHub Publication

The intended public remote is https://github.com/EiscremeWaffle/bubsy3d-decomp.
Only publish the repository subfolder, not the parent disc extraction.

After reviewing the files, make the first commit and push from this directory:

```powershell
git status --short
git add .gitignore README.md requirements.txt config tools tests docs .github
git diff --cached --stat
git commit -m "Set up USA Bubsy 3D research scaffold"
git push -u origin main
```

Authenticate through your browser or Git credential manager when prompted. Do not
paste credentials into chat or put them in source files. No commit or push is
performed automatically by the setup tools.

## Progress Integration

decomp.dev requires actual reports from your default branch before registration.
The scaffold-check workflow is not sufficient and intentionally emits no report.

1. Create relocatable target objects representing the full original code layout.
   With splat, `make_full_disasm_for_code: True` produces full assembly for C/C++
   segments that can be assembled into target objects.
2. Build base objects from reconstructed source using the established toolchain.
   Exclude assembly fallback functions from decompiled-function counts. Unwritten
   units may have no base object, but must still contribute to the total.
3. Generate `objdiff.json` with every unit, its target path and optional base path.
   Include both executables and relevant overlays or state a narrower scope clearly.
4. Run `objdiff-cli report generate -o build/report.json` using a pinned CLI version.
5. Add these steps after the real build/report steps in GitHub Actions:

```yaml
- name: Upload USA progress report
  uses: actions/upload-artifact@v4
  with:
    name: SLUS_001.10_report
    path: build/report.json
    if-no-files-found: error
```

Only upload report metadata, never the original game files or proprietary tools.
Decide how CI will legally obtain required inputs before enabling a build job.
Do not fabricate a percentage from the executable's total byte size, and do not
report an assembly-only rebuild as decompiled C.

## Register the Project

After a real report artifact exists on the default branch:

1. Log in with GitHub at https://decomp.dev and visit https://decomp.dev/manage/new.
2. As a repository admin, select `EiscremeWaffle/bubsy3d-decomp` and configure
   the name, PlayStation platform, report workflow, and version/artifact details
   offered by the form. Use `Bubsy 3D: Furbitten Planet (USA)` as the title.
3. Optionally install https://github.com/apps/decomp-dev on this repository for
   faster report updates and configurable pull-request progress comments.

Registration and GitHub app authorization require your account and are not done
by local setup. Avoid adding a badge claiming progress before the project exists.

Official guide: https://decomp.wiki/tools/decomp-dev