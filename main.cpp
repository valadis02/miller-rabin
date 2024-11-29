using System;
using System.Numerics;
using System.Threading;
using System.Threading.Tasks;

public static class PrimeExtensions
{
    private static ThreadLocal<Random> s_Gen = new ThreadLocal<Random>(() => new Random());

    private static Random Gen => s_Gen.Value;

    public static bool IsProbablyPrime(this BigInteger value, int witnesses = 10)
    {
        if (value <= 1)
            return false;

        if (witnesses <= 0)
            witnesses = 10;

        BigInteger d = value - 1;
        int s = 0;

        while (d % 2 == 0)
        {
            d /= 2;
            s += 1;
        }

        var tasks = new Task<bool>[witnesses];
        for (int i = 0; i < witnesses; i++)
        {
            tasks[i] = Task.Run(() =>
            {
                BigInteger a;
                do
                {
                    a = GenerateRandomBigInteger(2, value - 2);
                }
                while (a < 2 || a >= value - 2);

                BigInteger x = BigInteger.ModPow(a, d, value);
                if (x == 1 || x == value - 1)
                    return true;

                for (int r = 1; r < s; r++)
                {
                    x = BigInteger.ModPow(x, 2, value);
                    if (x == 1)
                        return false;
                    if (x == value - 1)
                        break;
                }

                return x == value - 1;
            });
        }

        Task.WhenAll(tasks).Wait();

        // If any of the tests returned false, this ain't prime
        foreach (var task in tasks)
        {
            if (!task.Result)
                return false;
        }

        return true;
    }

    private static BigInteger GenerateRandomBigInteger(BigInteger min, BigInteger max)
    {
        if (min >= max) throw new ArgumentException("min must be less than max");

        BigInteger range = max - min + 1;
        byte[] bytes = new byte[range.ToByteArray().Length];

        BigInteger randomValue;
        do
        {
            Gen.NextBytes(bytes);
            randomValue = new BigInteger(bytes) % range;
        } while (randomValue < 0 || randomValue >= range);

        return randomValue + min;
    }
}


class Program
{
    static void Main(string[] args)
    {
        // Example arbitrary numbers (not of the form 2^n - 1)
        BigInteger value1 = BigInteger.Parse("17");
        BigInteger value2 = BigInteger.Parse("1701411834604692317316873037158841057");
        // Example Mersenne primes (2^n - 1)
        BigInteger value3 = BigInteger.Pow(2, 1001) - 1; // Number (2^1001 - 1)
        BigInteger value4 = BigInteger.Pow(2, 23209) - 1; // Mersenne prime number (2^2281 - 1)
        BigInteger value5 = BigInteger.Pow(2, 44497) - 1; // Mersenne prime number (2^9941 - 1)
        BigInteger value6 = BigInteger.Pow(2, 110503) - 1; // Mersenne prime number (2^22701 - 1)


        MeasurePrimeTest(value1, 10);
        MeasurePrimeTest(value2, 10);
        MeasurePrimeTest(value3, 10);
        MeasurePrimeTest(value4, 10);
        MeasurePrimeTest(value5, 10);
        MeasurePrimeTest(value6, 10);
    }

    static void MeasurePrimeTest(BigInteger value, int witnesses)
    {
        var stopwatch = new System.Diagnostics.Stopwatch();
        stopwatch.Start();

        bool isPrime = value.IsProbablyPrime(witnesses);

        stopwatch.Stop();

        Console.WriteLine($"Number: {value}");
        Console.WriteLine(isPrime ? "probably prime" : "not prime");
        Console.WriteLine($"Elapsed Time: {stopwatch.ElapsedMilliseconds} ms");
        Console.WriteLine();
    }
}
