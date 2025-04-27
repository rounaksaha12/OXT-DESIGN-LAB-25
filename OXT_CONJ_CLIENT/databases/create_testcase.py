import random

def process_db6k(file_path, N):
    keyword_to_docs = {}  # Maps keyword id (line number) -> set of doc ids
    doc_to_keywords = {}  # Maps doc id -> set of keyword ids (line numbers)

    # Step 1: Parse the file and build mappings
    with open(file_path, 'r') as f:
        for idx, line in enumerate(f):
            parts = line.strip().split(',')
            keyword_docs = set(filter(None, parts[1:]))  # Filter out any empty strings
            keyword_to_docs[idx] = keyword_docs
            for doc in keyword_docs:
                doc_to_keywords.setdefault(doc, set()).add(idx)

    # Step 2: Find valid pairs (a, b) where:
    # - a != b
    # - len(keyword_to_docs[a]) != len(keyword_to_docs[b])
    # - keyword_to_docs[a] ∩ keyword_to_docs[b] is non-empty

    valid_pairs = set()
    for doc, keywords in doc_to_keywords.items():
        keywords = list(keywords)
        for i in range(len(keywords)):
            for j in range(i + 1, len(keywords)):
                a, b = keywords[i], keywords[j]
                # Enforce a < b for uniqueness
                if a > b:
                    a, b = b, a
                if len(keyword_to_docs[a]) != len(keyword_to_docs[b]):
                    valid_pairs.add((a, b))

    # Randomly sample N valid pairs
    sampled_pairs = random.sample(list(valid_pairs), min(N, len(valid_pairs)))

    # Step 3: Write to input.txt and exp_output.txt
    with open('input.txt', 'w') as f_in, open('exp_output.txt', 'w') as f_out:
        for a, b in sampled_pairs:
            common_docs = keyword_to_docs[a] & keyword_to_docs[b]
            if common_docs:
                f_in.write(f"{a},{b}\n")
                f_out.write(','.join(sorted(common_docs)) + '\n')

    print(f"Generated {len(sampled_pairs)} pairs in input.txt and exp_output.txt.")

# Example usage:
process_db6k('db6k.dat', 100)

