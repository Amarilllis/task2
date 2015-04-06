#include <vector>
#include <math.h>

extern "C" {
#include <Python.h>
}

//const double eps = std::numeric_limits<double>::epsilon();
//const double inf = std::numeric_limits<double>::infinity();

static PyObject * run_floyd(PyObject* module, PyObject* args)
{
	PyObject * a = PyTuple_GetItem(args, 0);
    PyObject * result = PyList_New(0);
	for (int k=0; k<PyObject_Length(a); ++k) {
		PyObject * res_row = PyList_New(0);
		for (int i=0; i<PyObject_Length(a); ++i) {
            PyObject * res_elem = PyFloat_FromDouble(0.0);

            for (int j=0; j<PyObject_Length(a); ++j) {
                if (i == j) {
                    continue;
                }

                PyObject * a_i = PySequence_GetItem(a, i);
				PyObject * a_ij = PySequence_GetItem(a_i, j);
                PyObject * a_ik = PySequence_GetItem(a_i, k);

                PyObject * a_k = PySequence_GetItem(a, i);
				PyObject * a_kj = PySequence_GetItem(a_k, j);
                
                PyObject * sum = PyNumber_Add(a_ik, a_kj);
                
                PyObject * one = PyFloat_FromDouble(1.0);

                PyObject * inv_sum = PyNumber_TrueDivide(one, sum);
                
                PyObject * inv_elem = PyNumber_TrueDivide(one, a_ij);
                
                
                PyObject * res = PyNumber_TrueDivide(one, PyNumber_Add(inv_elem, inv_sum));
                
                Py_DECREF(res);
                Py_DECREF(res_elem);
                res_elem = res;
			}
			PyList_Append(res_row, res_elem);
		}
		PyList_Append(result, res_row);
    }
	return result;
}

namespace floyd {
	typedef std::vector<double>  row_t;
	typedef std::vector<row_t>   matrix_t;

	static matrix_t myfloyd(const matrix_t &a)
	{
		matrix_t result;
		result.resize(a.size());
		for (size_t k=0; k<a.size(); ++k) {
			result[k].resize(a.size());
			for (size_t i=0; i<a.size(); ++i) {
				double res_elem = 0.0;
				for (size_t j=0; j<a.size(); ++j) {
					const double a_ij = a[i][j];
					const double a_ik = a[i][k];
                    const double a_kj = a[k][j];
                    res_elem = 1 / (1/a_ij + 1/(a_ik + a_kj));
                    
                    //d[i][j] = InvDiv(InvDiv(d[i][j]) + InvDiv(d[i][k]+d[k][j])
                    
                    result[i][j] = res_elem; //really?
				}
			}
		}
		return result;
	}
}

static floyd::matrix_t pyobject_to_cxx(PyObject * py_matrix)
{
	floyd::matrix_t result;
	result.resize(PyObject_Length(py_matrix));
	for (size_t i=0; i<result.size(); ++i) {
		PyObject * py_row = PyList_GetItem(py_matrix, i);
		floyd::row_t & row = result[i];
		row.resize(PyObject_Length(py_row));
		for (size_t j=0; j<row.size(); ++j) {
			PyObject * py_elem = PyList_GetItem(py_row, j);
			const double elem = PyFloat_AsDouble(py_elem);
			row[j] = elem;
		}
	}
	return result;
}

static PyObject * cxx_to_pyobject(const floyd::matrix_t &matrix)
{
	PyObject * result = PyList_New(matrix.size());
	for (size_t i=0; i<matrix.size(); ++i) {
		const floyd::row_t & row = matrix[i];
		PyObject * py_row = PyList_New(row.size());
		PyList_SetItem(result, i, py_row);
		for (size_t j=0; j<row.size(); ++j) {
			const double elem = row[j];
			PyObject * py_elem = PyFloat_FromDouble(elem);
			PyList_SetItem(py_row, j, py_elem);
		}
	}
	return result;
}



PyMODINIT_FUNC PyInit_floyd()
{
	static PyMethodDef ModuleMethods[] = {
		{ "Floyd", run_floyd, METH_VARARGS, "floyd(matrix):\n\n"
		"Floyd-Warshall algorithm" },
		{ NULL, NULL, 0, NULL }
	};
	static PyModuleDef ModuleDef = {
		PyModuleDef_HEAD_INIT,
		"floyd",
		"Floyd-Warshall",
		-1, ModuleMethods,
		NULL, NULL, NULL, NULL
	};
	PyObject * module = PyModule_Create(&ModuleDef);
	return module;
}
