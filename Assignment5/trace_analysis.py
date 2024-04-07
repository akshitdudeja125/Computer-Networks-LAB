import  traceanalyzer as tr

eedelay0=tr.Eedelay('Q1/first.tr','0')
eedelay1=tr.Eedelay('Q1/first.tr','1')
eedelay2=tr.Eedelay('Q1/first.tr','2')
eedelay3=tr.Eedelay('Q1/first.tr','3')

arr0 = eedelay0.eedelay_array
arr1 = eedelay1.eedelay_array
arr2 = eedelay2.eedelay_array
arr3 = eedelay3.eedelay_array

print("For Q1")
print("Average end-to-end delay for node 0 is " ,  sum (arr0) / len (arr0))
print("Average end-to-end delay for node 1 is " ,  sum (arr1) / len (arr1))
print("Average end-to-end delay for node 2 is " ,  sum (arr2) / len (arr2))
print("Average end-to-end delay for node 3 is " ,  sum (arr3) / len (arr3))


eedelay0=tr.Eedelay('Q2/second.tr','0')
eedelay1=tr.Eedelay('Q2/second.tr','1')
eedelay2=tr.Eedelay('Q2/second.tr','2')
eedelay3=tr.Eedelay('Q2/second.tr','3')

arr0 = eedelay0.eedelay_array
arr1 = eedelay1.eedelay_array
arr2 = eedelay2.eedelay_array
arr3 = eedelay3.eedelay_array

print("\nFor Q2")
print("Average end-to-end delay for node 0 is " ,  sum (arr0) / len (arr0))
print("Average end-to-end delay for node 1 is " ,  sum (arr1) / len (arr1))
print("Average end-to-end delay for node 2 is " ,  sum (arr2) / len (arr2))
print("Average end-to-end delay for node 3 is " ,  sum (arr3) / len (arr3))