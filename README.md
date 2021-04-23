# qpdf

> Split, Combine and Compress PDF files

[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](httsp://www.repostatus.org/badges/latest/active.svg)](https://www.repostatus.org/#active)
[![CRAN_Status_Badge](http://www.r-pkg.org/badges/version/qpdf)](http://cran.r-project.org/package=qpdf)
[![CRAN RStudio mirror downloads](http://cranlogs.r-pkg.org/badges/qpdf)](http://cran.r-project.org/web/packages/qpdf/index.html)

## Hello World

All functions take on or more input and output pdf files.

```{r}
> library(qpdf)
> pdf_compress("~/Downloads/v71i02.pdf")
[1] "/Users/jeroen/Downloads/v71i02_output.pdf"
```
