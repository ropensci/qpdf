pdf_create_test_stamp <- function() {
  path <- tempfile(fileext = ".pdf")
  grDevices::pdf(path)
  grid::grid.text("DRAFT", gp = grid::gpar(col = "red", fontsize = 72, alpha = 0.3))
  grDevices::dev.off()
  invisible(path)
}

test_that("pdf_overlay_stamp works", {
  pdf_file <- 'pdf-example-password.original.pdf'
  stamp_file <- pdf_create_test_stamp()
  outfile_all <- tempfile(fileext = ".pdf")
  outfile_one <- tempfile(fileext = ".pdf")
  on.exit(unlink(c(stamp_file, outfile_all, outfile_one)))

  # pdf_overlay_stamp should stamp all pages by default
  pdf_overlay_stamp(
    pdf_file,
    stamp_file,
    output = outfile_all,
    password = "test"
  )
  expect_equal(
    pdf_length(outfile_all, password = "test"),
    pdf_length(pdf_file, password = "test")
  )

  # pdf_overlay_stamp only stamps selected pages
  pdf_overlay_stamp(
    pdf_file,
    stamp_file,
    output = outfile_one,
    pages = 1,
    password = "test"
  )
  expect_equal(
    pdf_length(outfile_one, password = "test"),
    pdf_length(pdf_file, password = "test")
  )
  expect_false(
    tools::md5sum(outfile_all)[[1]] == tools::md5sum(outfile_one)[[1]]
  )

  # pdf_overlay_stamp errors on out-of-range pages
  expect_error(pdf_overlay_stamp(pdf_file, stamp_file, pages = 999, password = "test"))
})
