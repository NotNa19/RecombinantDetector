from fasta_reader import read_fasta
import sys, os
import random
input_par1=1
input_par2=50
input_par3=10
input_fasta="simulated.fasta"

 
seqs = {}
for h,s in read_fasta(input_fasta):
    seqs[h] = s
    temp_length.append(len(s))

if input_par1==0:## all recombinants are two-sources. eg:set 50 jump once seqs and remaining 50 are nonrecombinants.
    need_seqs_num=2*input_par2+200-input_par2
    seq_index=random.sample(range(need_seqs_num), k=2*input_par2)
    with open("seqs_recombined.fasta", 'w') as outfile:
        for count in range(input_par2):
            left_source=seq_index[2*count];
            right_source=seq_index[2*count+1]
            
            temp=min(len(seqs[str(left_source)]), len(seqs[str(right_source)]))
            bkp=random.sample(range(1,temp),1)[0]
            left_source_seq=seqs[str(left_source)][0:bkp]
            right_source_seq=seqs[str(right_source)][bkp:]
            
            outfile.write(">"+"r_"+str(left_source)+"_"+str(right_source)+"_bkp"+str(bkp)+"\n"+left_source_seq+right_source_seq+"\n")
        seq_index_unrecombined=list(set(range(need_seqs_num)) - set(seq_index))
        for count in seqs.keys():
            outfile.write(">"+ str(count)+"\n"+seqs[str(count)]+"\n")
else: ## two recombination points
    need_seqs_num=2*input_par2+200-input_par2
    seq_index=random.sample(list(seqs.keys()), k=2*input_par2)
    with open("seqs_recombined.fasta", 'w') as outfile:
        for count in range(input_par2):
            left_source=seq_index[2*count];
            right_source=seq_index[2*count+1]
            
            temp=min(len(seqs[str(left_source)]), len(seqs[str(right_source)]))
            bkp1=random.sample(range(1,int(temp/2)),1)[0]
            bkp2=random.sample(range(int(temp/2)+2, temp),1)[0]
            left_source_seq1=seqs[str(left_source)][0:bkp1]
            right_source_seq=seqs[str(right_source)][bkp1:bkp2]
            left_source_seq2=seqs[str(left_source)][bkp2:]
            
            outfile.write(">"+"r_double"+str(left_source)+"_"+str(right_source)+"_bkp"+str(bkp1)+"_bkp"+str(bkp2)+"\n"+left_source_seq1+right_source_seq+left_source_seq2+"\n")
        seq_index_unrecombined=list(set(range(need_seqs_num)) - set(seq_index))
        for count in seqs.keys():
            outfile.write(">"+ str(count)+"\n"+seqs[str(count)]+"\n")
            
