# musivid

```c
filesrc location=input.flac ! decodebin ! voaacenc ! outmux. filesrc location=cover.jpg ! jpegdec ! imagefreeze ! x264enc ! mp4mux name=outmux ! progressreport update-freq=1 ! filesink location=tst.mp4
```
