#' Split PDF
#'
#' Split a pdf file in multiple pages.
#'
#' @export
#' @rdname qpdf
#' @useDynLib qpdf
#' @importFrom Rcpp sourceCpp
#' @importFrom askpass askpass
#' @param input path to the input pdf file
#' @param output base path of the output file(s)
#' @param password string or callback function with password to open pdf
pdf_split <- function(input, output = NULL, password = ""){
  input <- normalizePath(input, mustWork = TRUE)
  if(!length(output))
    output <- sub("\\.pdf$", "", input)
  cpp_pdf_split(input, output, password)
}

#' @export
#' @rdname qpdf
pdf_length <- function(input, password = ""){
  input <- normalizePath(input, mustWork = TRUE)
  cpp_pdf_length(input, password)
}

#' @export
#' @rdname qpdf
#' @param pages a vector with page numbers so select. Negative numbers
#' means removing those pages (same as R indexing)
pdf_select <- function(input, pages = 1, output = NULL, password = ""){
  input <- normalizePath(input, mustWork = TRUE)
  if(!length(output))
    output <- sub("\\.pdf$", "_output.pdf", input)
  output <- normalizePath(output, mustWork = FALSE)
  size <- pdf_length(input)
  pages <- seq_len(size)[pages]
  if(any(is.na(pages)) || !length(pages))
    stop("Selected pages out of range")
  cpp_pdf_select(input, output, pages, password)
}

#' @export
#' @rdname qpdf
pdf_combine <- function(input, output = NULL, password = ""){
  input <- normalizePath(input, mustWork = TRUE)
  if(!length(output))
    output <- sub("\\.pdf$", "_combined.pdf", input[1])
  output <- normalizePath(output, mustWork = FALSE)
  cpp_pdf_combine(input, output, password)
}

#' @export
#' @rdname qpdf
#' @param linearize enable pdf linearization (streamable pdf)
pdf_compress <- function(input, output = NULL, linearize = FALSE, password = ""){
  input <- normalizePath(input, mustWork = TRUE)
  if(!length(output))
    output <- sub("\\.pdf$", "_output.pdf", input)
  output <- normalizePath(output, mustWork = FALSE)
  cpp_pdf_compress(input, output, linearize, password)
}

password_callback <- function(...){
  paste(askpass::askpass(...), collapse = "")
}
