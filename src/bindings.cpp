#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>
#include <string>
#include <Rcpp.h>

static QPDF read_pdf_with_password(char const* infile, char const* password){
  QPDF pdf;
  try {
    pdf.processFile(infile, password);
  } catch(const std::exception& e){
    if (strlen(password) == 0 && strstr(e.what(), "password") != NULL) {
      Rcpp::Function askpass = Rcpp::Environment::namespace_env("qpdf")["password_callback"];
      Rcpp::String value = askpass("Please enter password to open PDF file");

      //this is only for testing the new password */
      QPDF pdf2;
      pdf2.processFile(infile, value.get_cstring());

      //actually read it
      pdf.processFile(infile, value.get_cstring());
    } else {
      throw;
    }
  }
  return pdf;
}

// [[Rcpp::export]]
int cpp_pdf_length(char const* infile, char const* password){
  QPDF pdf = read_pdf_with_password(infile, password);
  QPDFObjectHandle root = pdf.getRoot();
  QPDFObjectHandle pages = root.getKey("/Pages");
  QPDFObjectHandle count = pages.getKey("/Count");
  return count.getIntValue();
}

// [[Rcpp::export]]
Rcpp::CharacterVector cpp_pdf_split(char const* infile, std::string outprefix, char const* password){
  QPDF inpdf = read_pdf_with_password(infile, password);
  std::vector<QPDFPageObjectHelper> pages =  QPDFPageDocumentHelper(inpdf).getAllPages();
  Rcpp::CharacterVector output(pages.size());
  for (int i = 0; i < pages.size(); i++) {
    std::string outfile = outprefix + "_" + QUtil::int_to_string(i+1) + ".pdf";
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
Rcpp::CharacterVector cpp_pdf_select(char const* infile, char const* outfile,
                                     Rcpp::IntegerVector which, char const* password){
  QPDF inpdf = read_pdf_with_password(infile, password);
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
Rcpp::CharacterVector cpp_pdf_combine(Rcpp::CharacterVector infiles, char const* outfile,
                                      char const* password){
  QPDF outpdf;
  outpdf.emptyPDF();
  for (int i = 0; i < infiles.size(); i++) {
    QPDF inpdf = read_pdf_with_password(infiles.at(i), password);
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
Rcpp::CharacterVector cpp_pdf_compress(char const* infile, char const* outfile,
                                       bool linearize, char const* password){
  QPDF inpdf = read_pdf_with_password(infile, password);
  QPDFWriter outpdfw(inpdf, outfile);
  outpdfw.setStaticID(true); // for testing only
  outpdfw.setStreamDataMode(qpdf_s_compress);
  outpdfw.setLinearization(linearize);
  outpdfw.write();
  return outfile;
}
