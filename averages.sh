#!/usr/bin/env bash

output_file="data/averages.txt"
> "$output_file"

results=()

for file in ./data/*; do
    [[ -f "$file" ]] || continue

    # Extract numbers after the header line
    values=$(awk '/Elapsed times \(seconds\):/{flag=1; next} flag && NF' "$file")

    if [[ -n "$values" ]]; then
        sum=$(echo "$values" | awk '{s+=$1} END {print s}')
        count=$(echo "$values" | wc -l)
        avg=$(awk -v s="$sum" -v c="$count" 'BEGIN {printf "%.6f", s/c}')

        base=$(basename "$file")
        type=$(echo "$base" | cut -d'_' -f2)
        value=$(echo "$base" | cut -d'_' -f3-)
        value=${value%.log}

        key="$type $value:"
        results+=("$key $avg")
    fi
done

# Sort and align so that the decimal points line up
printf "%s\n" "${results[@]}" | sort -k1,1 -k2,2n | awk '
{
    key = ""
    for (i = 1; i < NF; i++) key = key " " $i
    sub(/^ /, "", key)
    sub(/:$/, ":", key)
    val = $NF
    split(val, parts, ".")
    intlen = length(parts[1])
    if (intlen > maxint) maxint = intlen
    if (length(key) > maxkey) maxkey = length(key)
    labels[NR] = key
    ints[NR] = parts[1]
    fracs[NR] = parts[2]
}
END {
    for (i = 1; i <= NR; i++) {
        padkey = sprintf("%-*s", maxkey + 2, labels[i])
        padint = sprintf("%*s", maxint, ints[i])
        printf "%s%s.%s seconds\n", padkey, padint, fracs[i]
    }
}' > "$output_file"
