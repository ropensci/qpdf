#' Split PDF
#'
#' Split a pdf file in multiple pages.
#'
#' @export
#' @rdname qpdf
#' @useDynLib qpdf
#' @importFrom Rcpp sourceCpp
#' @param input path to the input pdf file
#' @param output base path of the output file(s)
pdf_split <- function(input, output = NULL){
  input <- normalizePath(input, mustWork = TRUE)
  if(!length(output))
    output <- sub("\\.pdf$", "", input)
  cpp_pdf_split(input, output)
}

#' @export
#' @rdname qpdf
pdf_length <- function(input){
  input <- normalizePath(input, mustWork = TRUE)
  cpp_pdf_length(input)
}

#' @export
#' @rdname qpdf
pdf_select <- function(input, pages = 1, output = NULL){
  input <- normalizePath(input, mustWork = TRUE)
  if(!length(output))
    output <- sub("\\.pdf$", "_output.pdf", input)
  output <- normalizePath(output, mustWork = FALSE)
  size <- pdf_length(input)
  pages <- seq_len(size)[pages]
  if(any(is.na(pages)) || !length(pages))
    stop("Selected pages out of range")
  cpp_pdf_select(input, output, pages)
}

#' @export
#' @rdname qpdf
pdf_compress <- function(input, output = NULL){
  input <- normalizePath(input, mustWork = TRUE)
  if(!length(output))
    output <- sub("\\.pdf$", "_output.pdf", input)
  output <- normalizePath(output, mustWork = FALSE)
  cpp_pdf_compress(input, output)
}
