context("test rotate pages")

test_that("pdf_rotate_pages works", {
  pdf_file <- 'pdf-example-password.original.pdf'
  ## rotating by 90 and by -270 should give the same result
  outfile1 <- tempfile(fileext = ".pdf")
  pdf_rotate_pages(pdf_file, pages = 1, angle = 90, output = outfile1, password = "test")
  outfile2 <- tempfile(fileext = ".pdf")
  pdf_rotate_pages(pdf_file, pages = 1, angle = -270, output = outfile2, password = "test")
  expect_identical(unname(tools::md5sum(outfile1)), unname(tools::md5sum(outfile2)))
  ## but should be different to the input file
  expect_false(unname(tools::md5sum(outfile1)) == unname(tools::md5sum(pdf_file)))
  unlink(outfile1)
  unlink(outfile2)
})
