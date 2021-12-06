#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

// function to thread
void insert_sort(std::vector<int> &vec, size_t start, size_t length) {
    for (size_t i = start + 1; i < length + start; ++i){
		for (size_t j = i; (j > start) && (vec[j - 1] > vec[j]); --j) {
			int tmp = vec[j - 1];
			vec[j - 1] = vec[j];
			vec[j] = tmp;
		}
	}
}

// function to thread
void merge(std::vector<int> &vec, size_t start1, size_t len1, size_t start2, size_t len2) {
    int tmp[len1 + len2];
    size_t beg = start1;
    size_t cur1 = start1, cur2 = start2;
    for (size_t i = 0; i < len1 + len2; ++i) {
        if ((vec[cur1] <= vec[cur2]) && (cur1 < start2) && (cur2 < start2 + len2)) {
            tmp[i] = vec[cur1];
            ++cur1;
        }
        else if ((vec[cur1] > vec[cur2]) && (cur1 < start2) && (cur2 < start2 + len2)) {
            tmp[i] = vec[cur2];
            ++cur2;
        }
        else if ((cur1 < start2) && (cur2 == start2 + len2)) {
            tmp[i] = vec[cur1];
            ++cur1;
        }
        else if ((cur1 == start2) && (cur2 < start2 + len2)) {
            tmp[i] = vec[cur2];
            ++cur2;
        }
    }
    for (size_t i = 0; i < len1 + len2; ++i) {
        vec[beg] = tmp[i];
        ++beg;
    }
}

size_t GetMinrun(size_t n)
{
	size_t r = 0;
    while (n >= 64) {
	    r |= n & 1;
	    n >>= 1;
	}
	return n + r;
}

void TimSort(std::vector<int> &vec, size_t n) {
    size_t minrun = GetMinrun(vec.size());
    std::cout << "MINRUN = " << minrun << std::endl;
    std::vector<int> runs;
    size_t cur = 0, tmp = 0;
    while (cur < vec.size() - 1) {
        size_t run_cur = cur;
        if (vec[run_cur] > vec[run_cur + 1]) {
            tmp = vec[run_cur];
            vec[run_cur] = vec[run_cur + 1];
            vec[run_cur + 1] = tmp;
        }
        bool right = true;
        while (right) {
            // если не соблюдается порядок возрастания
            if (vec[run_cur] > vec[run_cur + 1])
                right = false;
            // но минран ещё не достигнут
            if (!(right) && (run_cur - cur < minrun))
                right = true;
            // конец вектора
            if (run_cur == vec.size() - 1)
                right = false;
   
            if (right)
                ++run_cur;
        }
        size_t size = run_cur - cur + 1;

        runs.push_back(cur);
        runs.push_back(size);

        cur = run_cur + 1;
    }

    std::thread thr[n];
    size_t c = 0;

    for (size_t i = 0; i < runs.size() - 1; i += 2) {
        if (c < n) {
            thr[c] = std::thread(insert_sort, std::ref(vec), runs[i], runs[i + 1]);
            ++c;
        } else {
            for (size_t j = 0; j < c; ++j) {
                thr[j].join();
            }
            c = 0;
            thr[c] = std::thread(insert_sort, std::ref(vec), runs[i], runs[i + 1]);
            ++c;
        }
    }

    for (size_t i = 0; i < c; ++i) {
        thr[i].join();
    }

    // merge
    while(runs.size() > 2) {
        size_t k = 0;
        for (size_t i = 0; i < runs.size(); i += 4) {
            int x_start = runs[i], x_len = runs[i + 1];
            int y_start = runs[i + 2], y_len = runs[i + 3];
            if (k < n) {
                thr[k] = std::thread(merge, std::ref(vec), x_start, x_len, y_start, y_len);
                ++k;
            } else {
                for (size_t j = 0; j < k; ++j)
                    thr[j].join();
                k = 0;
                thr[k] = std::thread(merge, std::ref(vec), x_start, x_len, y_start, y_len);
                ++k;
            }
            x_len += y_len;
            runs[i + 1] = x_len;
            runs[i + 2] = -1;
            runs[i + 3] = -1;
        }
        
        for (int j = runs.size() - 1; j >= 0; --j) {
            if (runs[j] == -1) {
                runs.erase(runs.begin() + j);
            }
        }

        for (size_t i = 0; i < k; ++i) {
            thr[i].join();
        }
    }
}

int main(int argc, char* argv[]) {
    int m, n;
    if ((argc != 2) || (atoi(argv[1]) < 1)) {
        std::cout << "Invalid number" << std::endl;
        exit(EXIT_FAILURE);
    }
    n = strtol(argv[1], NULL, 10);
    std::cout << "Max threads number is " << n << std::endl;
    std::vector<int> forsort;
    std::cout << "Enter vector for sorting:" << std::endl;
    while (std::cin >> m) {
        forsort.push_back(m);
    }
    std::cout << "Size = " << forsort.size() << std::endl;
    if (forsort.size() < 64) {
        insert_sort(forsort, 0, forsort.size());
    } else {
        auto t1 = std::chrono::high_resolution_clock::now();
        TimSort(forsort, n);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        std::cout << "Time: " << duration << " microseconds" << std::endl;
    }
    std::cout << "Sorted vector:" << std::endl;
    for (size_t i = 0; i < forsort.size(); ++i) {
        std:: cout << forsort[i] << " ";
    }
    std::cout << std::endl;
}