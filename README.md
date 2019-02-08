# qpdf

> Split, Combine and Compress PDF files

[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)
[![Build Status](https://travis-ci.org/ropensci/qpdf.svg?branch=master)](https://travis-ci.org/ropensci/qpdf)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/ropensci/qpdf?branch=master&svg=true)](https://ci.appveyor.com/project/jeroen/qpdf)
[![Coverage Status](https://codecov.io/github/ropensci/qpdf/coverage.svg?branch=master)](https://codecov.io/github/ropensci/qpdf?branch=master)
[![CRAN_Status_Badge](http://www.r-pkg.org/badges/version/qpdf)](http://cran.r-project.org/package=qpdf)
[![CRAN RStudio mirror downloads](http://cranlogs.r-pkg.org/badges/qpdf)](http://cran.r-project.org/web/packages/qpdf/index.html)

## Hello World

All functions take on or more input and output pdf files.

```{r}
> library(qpdf)
> pdf_compress("~/Downloads/v71i02.pdf")
[1] "/Users/jeroen/Downloads/v71i02_output.pdf"
```
