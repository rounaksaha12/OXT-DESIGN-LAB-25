def evaluate_results(exp_output_path, output_path):
    with open(exp_output_path, 'r') as f_exp, open(output_path, 'r') as f_out:
        exp_lines = f_exp.readlines()
        out_lines = f_out.readlines()

    # assert len(exp_lines) == len(out_lines), "Mismatch in number of lines between expected and actual output."
    print(len(exp_lines))
    print(len(out_lines))
    l = min(len(exp_lines), len(out_lines))
    
    exp_lines = exp_lines[:l]
    out_lines = out_lines[:l]

    total = len(exp_lines)
    correct = 0
    incorrect_cases = []

    for idx, (exp_line, out_line) in enumerate(zip(exp_lines, out_lines)):
        exp_docs = set(int(doc, 16) for doc in exp_line.strip().split(',') if doc)
        out_docs = set(int(doc, 16) for doc in out_line.strip().split(',') if doc)

        if exp_docs == out_docs:
            correct += 1
        else:
            incorrect_cases.append({
                'line': idx,
                'expected': sorted(hex(d) for d in exp_docs),
                'actual': sorted(hex(d) for d in out_docs)
            })

    accuracy = (correct / total) * 100
    print(f"Correct: {correct}/{total} ({accuracy:.2f}%)")

    if incorrect_cases:
        print("\nIncorrect Cases:")
        for case in incorrect_cases:
            print(f"Line {case['line']}:")
            print(f"  Expected: {', '.join(case['expected'])}")
            print(f"  Actual  : {', '.join(case['actual'])}")
    else:
        print("All outputs matched the expected results!")

evaluate_results('./results/exp_output.txt', './results/res_id.csv')

