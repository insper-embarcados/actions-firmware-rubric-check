import argparse
import yaml
import re
import sys

def check_code_against_rubric(c_file_path, rubric_file_path):
    """
    Checks a C code file against a rubric defined in a YAML file.

    Args:
        c_file_path (str): Path to the main.c file.
        rubric_file_path (str): Path to the rubric.yml file.
    """

    try:
        with open(rubric_file_path, 'r') as f:
            rubric = yaml.safe_load(f)
    except FileNotFoundError:
        print(f"Error: Rubric file not found at {rubric_file_path}")
        return
    except yaml.YAMLError as e:
        print(f"Error: Could not parse rubric file: {e}")
        return

    try:
        with open(c_file_path, 'r') as f:
            code_lines = f.readlines()
    except FileNotFoundError:
        print(f"Error: C code file not found at {c_file_path}")
        return

    erro = 0

    uncommented_code_lines = []
    in_multi_line_comment = False
    for line in code_lines:
        line = line.strip()
        if line.startswith('//'):
            continue  # Skip single-line comments

        if '/*' in line:
            in_multi_line_comment = True
            # Check if the multi-line comment ends on the same line
            if '*/' in line:
                in_multi_line_comment = False
            continue

        if '*/' in line:
            in_multi_line_comment = False
            continue

        if not in_multi_line_comment:
            uncommented_code_lines.append(line)

    uncommented_code_str = "\n".join(uncommented_code_lines)

    # Check for forbidden items
    if 'forbiten' in rubric:
        for item in rubric['forbiten']:
            if 'names' in item and 'error_text' in item:
                for name in item['names']:
                    if re.search(r'\b' + re.escape(name) + r'\b', uncommented_code_str):
                        print(f"ERRO: {item['error_text']}\n \t - Especificamente: {name}")
                        erro = erro + 1

    # Check for shall_have items
    if 'shall_have' in rubric:
        for item in rubric['shall_have']:
            if 'names' in item and 'error_text' in item:
                missing_names = []
                for declaration in item['names']:
                    if not re.search(r'\b' + re.escape(declaration) + r'\b', uncommented_code_str):
                        missing_names.append(declaration)

                if missing_names:
                    print(f"ERRO: {item['error_text']}")
                    for missing_name in missing_names:
                        print(f"\t - {missing_name}")
                        erro = erro + 1

    return erro

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Checks a C code file against a rubric.")
    parser.add_argument("c_file", help="Path to the main.c file")
    parser.add_argument("rubric_file", help="Path to the rubric.yml file")

    args = parser.parse_args()

    erro = check_code_against_rubric(args.c_file, args.rubric_file)
    sys.exit(erro)
