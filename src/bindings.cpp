#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>
#include <string>
#include <Rcpp.h>

// [[Rcpp::export]]
int cpp_pdf_length(char const* infile){
  QPDF pdf;
  pdf.processFile(infile);
  QPDFObjectHandle root = pdf.getRoot();
  QPDFObjectHandle pages = root.getKey("/Pages");
  QPDFObjectHandle count = pages.getKey("/Count");
  return count.getIntValue();
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
    outpdfw.setStreamDataMode(qpdf_s_preserve);
    outpdfw.write();
  }
  return output;
}

// [[Rcpp::export]]
Rcpp::CharacterVector cpp_pdf_select(char const* infile, char const* outfile, Rcpp::IntegerVector which){
  QPDF inpdf;
  inpdf.processFile(infile);
  std::vector<QPDFPageObjectHelper> pages =  QPDFPageDocumentHelper(inpdf).getAllPages();
  QPDF outpdf;
  outpdf.emptyPDF();
  for (int i = 0; i < which.size(); i++) {
    int index = which.at(i) -1; //zero index
    QPDFPageDocumentHelper(outpdf).addPage(pages.at(index), false);
  }
  QPDFWriter outpdfw(outpdf, outfile);
  outpdfw.setStaticID(true); // for testing only
  outpdfw.setStreamDataMode(qpdf_s_preserve);
  outpdfw.write();
  return outfile;
}

// [[Rcpp::export]]
Rcpp::CharacterVector cpp_pdf_combine(Rcpp::CharacterVector infiles, char const* outfile){
  QPDF outpdf;
  outpdf.emptyPDF();
  for (int i = 0; i < infiles.size(); i++) {
    QPDF inpdf;
    inpdf.processFile(infiles.at(i));
    std::vector<QPDFPageObjectHelper> pages =  QPDFPageDocumentHelper(inpdf).getAllPages();
    for (int i = 0; i < pages.size(); i++) {
      QPDFPageDocumentHelper(outpdf).addPage(pages.at(i), false);
    }
  }
  QPDFWriter outpdfw(outpdf, outfile);
  outpdfw.setStaticID(true); // for testing only
  outpdfw.setStreamDataMode(qpdf_s_preserve);
  outpdfw.write();
  return outfile;
}

// [[Rcpp::export]]
Rcpp::CharacterVector cpp_pdf_compress(char const* infile, char const* outfile){
  QPDF inpdf;
  inpdf.processFile(infile);
  QPDFWriter outpdfw(inpdf, outfile);
  outpdfw.setStaticID(true); // for testing only
  outpdfw.setStreamDataMode(qpdf_s_compress);
  outpdfw.write();
  return outfile;
}
