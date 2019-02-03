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
split_pdf <- function(input, output = NULL){
  input <- normalizePath(input, mustWork = TRUE)
  if(!length(output))
    output <- sub("\\.pdf$", "", input)
  cpp_split_pdf(input, output)
}
