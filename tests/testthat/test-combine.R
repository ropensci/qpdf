test_that("pdf_combine works with many files (no 'too many open files' error)", {
  pdf_file <- "pdf-example-password.original.pdf"
  pages_per_file <- pdf_length(pdf_file, password = "test")

  outfile_2 <- tempfile(fileext = ".pdf")
  on.exit(unlink(outfile_2), add = TRUE)
  pdf_combine(rep_len(pdf_file, 2), outfile_2, password = "test")
  expect_equal(pdf_length(outfile_2, password = "test"), 2 * pages_per_file)

  skip("May take 40+ seconds and create a 31Mb temporary file")
  n <- 10000L
  blank_file <- tempfile(fileext = ".pdf")
  outfile_n <- tempfile(fileext = ".pdf")
  on.exit(unlink(c(blank_file, outfile_n)), add = TRUE)
  grDevices::pdf(blank_file)
  grid::grid.newpage()
  invisible(grDevices::dev.off())
  pdf_combine(rep_len(blank_file, n), outfile_n)
  expect_equal(pdf_length(outfile_n), n)
})
