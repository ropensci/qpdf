#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>
#include <string>
#include <Rcpp.h>

// [[Rcpp::export]]
int cpp_pdf_length(char const* infile){
  QPDF inpdf;
  inpdf.processFile(infile);
  std::vector<QPDFPageObjectHelper> pages =  QPDFPageDocumentHelper(inpdf).getAllPages();
  return pages.size();
}

// [[Rcpp::export]]
Rcpp::CharacterVector cpp_pdf_split(char const* infile, std::string outprefix){
  QPDF inpdf;
  inpdf.processFile(infile);
  std::vector<QPDFPageObjectHelper> pages =  QPDFPageDocumentHelper(inpdf).getAllPages();
  Rcpp::CharacterVector output(pages.size());
  for (int i = 0; i < pages.size(); i++) {
    std::string outfile = outprefix + "_" + QUtil::int_to_string(i+1, pages.size()) + ".pdf";
    output.at(i) = outfile;
    QPDF outpdf;
    outpdf.emptyPDF();
    QPDFPageDocumentHelper(outpdf).addPage(pages.at(i), false);
    QPDFWriter outpdfw(outpdf, outfile.c_str());
    outpdfw.setStaticID(true); // for testing only
    outpdfw.setStreamDataMode(qpdf_s_uncompress);
    outpdfw.write();
  }
  return output;
}

// [[Rcpp::export]]
Rcpp::CharacterVector cpp_pdf_select(char const* infile, std::string outfile, Rcpp::IntegerVector which){
  QPDF inpdf;
  inpdf.processFile(infile);
  std::vector<QPDFPageObjectHelper> pages =  QPDFPageDocumentHelper(inpdf).getAllPages();
  QPDF outpdf;
  outpdf.emptyPDF();
  for (int i = 0; i < which.size(); i++) {
    int index = which.at(i) -1; //zero index
    QPDFPageDocumentHelper(outpdf).addPage(pages.at(index), false);
  }
  QPDFWriter outpdfw(outpdf, outfile.c_str());
  outpdfw.setStaticID(true); // for testing only
  outpdfw.setStreamDataMode(qpdf_s_uncompress);
  outpdfw.write();
  return outfile;
}
