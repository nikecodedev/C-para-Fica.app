# Splash Screen Specification

**Design:** Sleek, minimal, premium, authoritative. Fast load.

## Layout

```
┌─────────────────────────────────┐
│                                 │
│                                 │
│                                 │
│         [OFFICIAL LOGO]         │
│           Centered              │
│                                 │
│                                 │
│                                 │
│                                 │
│  © 2025 Fica Motor. All rights. │
└─────────────────────────────────┘
```

- **Background:** #0A0A0F (dark, premium)
- **Logo:** White or brand accent, centered vertically + horizontally
- **Copyright:** Bottom, 16pt from edge, #6B6B7B (muted)
- **Duration:** Min display 0.5s, max 2s (proceed when app ready)
- **No animation** on logo—instant, professional

## Assets

- `logo.png` / `logo@2x.png` / `logo@3x.png` — Place in platform resources
- Fallback: App name text if logo missing
