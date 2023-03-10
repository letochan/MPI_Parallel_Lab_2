#define MSMPI_NO_DEPRECATE_20
#define max_value 10

#define maxNtoView 15

#include <iostream>
#include <random>
#include <chrono>
#include "mpi.h"

using namespace std;

double *vect(int num, bool yep)   //создаем вектор
{
	double *data = new double[num];
	if (yep) //если инициализируем
	{
		srand(time(0));
		for (int i = 0; i < num; i++)
		data[i] = (rand() % 10) - 10;
	}
	else  //иначе зануляем
		for (int i = 0; i < num; i++)
			data[i] = 0;
	return data;
}

void outmas(double *mas, int length) //вывод вектора
{
	int n = length;
	if (length > maxNtoView)
			n = maxNtoView;
	for (int i = 0; i < n; i++)
	{
		cout << mas[i] << " ";
	}
	cout << endl;
}

bool compare(double *arr1, double *arr2, int length) //сравнение
{
	bool result = true;
	for (int i = 0; i < length; i++)
	{
		if (arr1[i] != arr2[i])
		{
			result = false;
			break;
		}
	}

	return result;

}
void MPI_Scatter(void *sbuf, int scount, MPI_Datatype stype, void *rbuf, int rcount, MPI_Datatype rtype, int root, MPI_Comm comm)
{
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Aint sendExtent;
	MPI_Aint recvExtent;

	MPI_Type_extent(stype, &sendExtent);
	MPI_Type_extent(rtype, &recvExtent);
	MPI_Status status;
	if (rank == root)
	{

		for (int i = 0; i < size; i++)
		{
			if (i != root)
			{
				MPI_Send((char *)sbuf + i * rcount * sendExtent, rcount, stype, i, 0, comm);
			}
			else
			for (int i = 0; i < rcount * sendExtent; i++)
				*((char *)(rbuf)+i) = *((char *)(sbuf)+root*rcount*sendExtent + i);
		}
		
	}

	else
	{
		MPI_Recv(rbuf, rcount, rtype, root, 0, comm, &status);
	}
}


int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv); //инициализация
	int rank = -1;
	int size = -1;
	int root=1;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	double *arr; //исходный вектор
	double *result; //результирующий вектор
	MPI_Status Status; //переменная статуса выполнения операции приема данных
	int length = 16;
	if (rank == 0)
	{
		cout << "\n Enter the length of vector:\n";
		cin >> length;
		cout << "\n Enter the number of root process:\n";
		cin >> root;
	}
	MPI_Bcast(&length,1,MPI_INT,0,MPI_COMM_WORLD);

	MPI_Bcast(&root, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int width = length / size;//длина кусочков
	double *tmp = vect(width,false);//вектор для передачи 
	arr = vect(length, true);//вектор изначальный
	result = vect(width, false); //вектор результирующий
	double startTime, endTime, stime;//переменные для подсчёта времени


	srand(time(0));


	if (rank == root) 
	{
		
		startTime = MPI_Wtime();
		cout << "\nYour vector is:\n";
		if (length>maxNtoView)
			outmas(arr, length);//выводим вектор
	}
	MPI_Scatter(arr, length, MPI_DOUBLE, tmp, width, MPI_DOUBLE, root, MPI_COMM_WORLD); //раздаём вектор 

	MPI_Gather(tmp, width, MPI_DOUBLE, result, width, MPI_DOUBLE, root, MPI_COMM_WORLD);//собираем вектор для сравнения

	if (rank == root) 
	{
		endTime = MPI_Wtime();
		stime = startTime - endTime;
		cout << "\nGather return this vector: \n";
			outmas(result, length);
		if (compare(arr, result, length) == true)
		{
			cout << "\nVector compared, all working correctly \n";
		}
		else cout << "Vector isn't compared, here is an error\n" << endl;
		cout << "\nScatter time : " << stime << "\n";
	}

	delete arr, result, tmp; //очищаем память
	MPI_Finalize(); //завершаем работу с MPI
	return 0;
}