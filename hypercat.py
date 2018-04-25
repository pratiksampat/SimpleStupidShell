import PyPDF2
import sys

def read_pdf(filename):
	# print("filename is ", filename)
	file_obj = open(filename, "rb")
	pdf_reader = PyPDF2.PdfFileReader(file_obj)

	num_pages = pdf_reader.numPages

	for page_nr in range(num_pages):
		
		this_page = pdf_reader.getPage(page_nr)

		print(this_page.extractText())

	file_obj.close()

if __name__=="__main__":
	read_pdf(sys.argv[1])