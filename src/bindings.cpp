#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>
#include <string>
#include <Rcpp.h>

static void read_pdf_with_password(char const* infile, char const* password, QPDF *pdf){
  try {
    pdf->processFile(infile, password);
  } catch(const std::exception& e){
    if (strlen(password) == 0 && strstr(e.what(), "password") != NULL) {
      Rcpp::Function askpass = Rcpp::Environment::namespace_env("qpdf")["password_callback"];
      Rcpp::String value = askpass("Please enter password to open PDF file");

      //this is only for testing the new password */
      QPDF pdf2;
      pdf2.processFile(infile, value.get_cstring());

      //actually read it
      pdf->processFile(infile, value.get_cstring());
    } else {
      throw;
    }
  }
}

// [[Rcpp::export]]
int cpp_pdf_length(char const* infile, char const* password){
  QPDF pdf;
  read_pdf_with_password(infile, password, &pdf);
  QPDFObjectHandle root = pdf.getRoot();
  QPDFObjectHandle pages = root.getKey("/Pages");
  QPDFObjectHandle count = pages.getKey("/Count");
  return count.getIntValue();
}

// [[Rcpp::export]]
Rcpp::CharacterVector cpp_pdf_split(char const* infile, std::string outprefix, char const* password){
  QPDF inpdf;
  read_pdf_with_password(infile, password, &inpdf);
  std::vector<QPDFPageObjectHelper> pages =  QPDFPageDocumentHelper(inpdf).getAllPages();
  Rcpp::CharacterVector output(pages.size());
  for (int i = 0; i < pages.size(); i++) {
    int countlen = ceil(log10(pages.size() + 1));
    std::string outfile = outprefix + "_" + QUtil::int_to_string(i+1, countlen) + ".pdf";
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
  QPDF inpdf;
  read_pdf_with_password(infile, password, &inpdf);
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
    QPDF inpdf;
    read_pdf_with_password(infiles.at(i), password, &inpdf);
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
  QPDF inpdf;
  read_pdf_with_password(infile, password, &inpdf);
  QPDFWriter outpdfw(inpdf, outfile);
  outpdfw.setStaticID(true); // for testing only
  outpdfw.setStreamDataMode(qpdf_s_compress);
  outpdfw.setLinearization(linearize);
  outpdfw.write();
  return outfile;
}

// [[Rcpp::export]]
Rcpp::CharacterVector cpp_pdf_rotate_pages(char const* infile, char const* outfile,
                                           Rcpp::IntegerVector which, int angle, bool relative,
                                           char const* password){
  QPDF inpdf;
  read_pdf_with_password(infile, password, &inpdf);
  std::vector<QPDFPageObjectHelper> pages =  QPDFPageDocumentHelper(inpdf).getAllPages();
  int npages = pages.size();
  QPDF outpdf;
  outpdf.emptyPDF();
  for (int i = 0; i < npages; i++) {
    if (std::find(which.begin(), which.end(), i + 1) != which.end()) {
      pages.at(i).rotatePage(angle, relative);
    }
    QPDFPageDocumentHelper(outpdf).addPage(pages.at(i), false);
  }
  QPDFWriter outpdfw(outpdf, outfile);
  outpdfw.setStaticID(true); // for testing only
  outpdfw.setStreamDataMode(qpdf_s_preserve);
  outpdfw.write();
  return outfile;
}

// [[Rcpp::export]]
Rcpp::CharacterVector cpp_pdf_overlay(char const* infile, char const* stampfile,
                                      char const* outfile, char const* password){
  QPDF inpdf;
  QPDF stamppdf;
  read_pdf_with_password(infile, password, &inpdf);
  read_pdf_with_password(stampfile, password, &stamppdf);

  // Code from: https://github.com/qpdf/qpdf/blob/release-qpdf-8.4.0/examples/pdf-overlay-page.cc
  QPDFPageObjectHelper stamp_page_1 =  QPDFPageDocumentHelper(stamppdf).getAllPages().at(0);
  QPDFObjectHandle foreign_fo = stamp_page_1.getFormXObjectForPage();
  QPDFObjectHandle stamp_fo = inpdf.copyForeignObject(foreign_fo);
  std::vector<QPDFPageObjectHelper> pages = QPDFPageDocumentHelper(inpdf).getAllPages();
  for (std::vector<QPDFPageObjectHelper>::iterator iter = pages.begin(); iter != pages.end(); ++iter) {
    QPDFPageObjectHelper& ph = *iter;
    QPDFObjectHandle resources = ph.getAttribute("/Resources", true);
    int min_suffix = 1;
    std::string name = resources.getUniqueResourceName("/Fx", min_suffix);
    std::string content =
      ph.placeFormXObject(
        stamp_fo, name, ph.getTrimBox().getArrayAsRectangle());
    if (! content.empty()) {
      resources.mergeResources(
        QPDFObjectHandle::parse("<< /XObject << >> >>"));
      resources.getKey("/XObject").replaceKey(name, stamp_fo);
      ph.addPageContents(
        QPDFObjectHandle::newStream(&inpdf, "q\n"), true);
      ph.addPageContents(
        QPDFObjectHandle::newStream(&inpdf, "\nQ\n" + content), false);
    }
  }
  QPDFWriter w(inpdf, outfile);
  w.setStaticID(true);        // for testing only
  w.write();
  return outfile;
}

