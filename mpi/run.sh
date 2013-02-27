for i in `seq -f "%02g" 20`
do
	make zin > e2e.100s.${i} 2>&1
done

