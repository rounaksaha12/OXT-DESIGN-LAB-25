with open("exp_output.txt","r") as fin:
	for idx, line in enumerate(fin):
		print(f"{idx} Nmatch: {len(line.split(","))}")
