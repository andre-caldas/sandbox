#ifndef FFTINPLACEARRAY_H
#define FFTINPLACEARRAY_H

#include <cassert>
#include <memory>

#include <complex.h> // So we can treat `fftw_complex` as a native complex type.
#include <fftw3.h>


class FFTInplaceArray
{
  private:
    class PlanAndBuffer {
      public:
        fftw_plan m_Plan;
        fftw_plan m_InversePlan;
        std::complex<double>* m_Buffer;

        PlanAndBuffer(size_t size);
        ~PlanAndBuffer(void);

        void execute (void) { fftw_execute(m_Plan); }
        void execute_inverse (void) { fftw_execute(m_InversePlan); }
    };


  public:
    FFTInplaceArray(void);
    FFTInplaceArray(size_t minimum_size);
    ~FFTInplaceArray(void);

    void init(size_t minimum_size);


    int get_lag_from (FFTInplaceArray &from, double* quality = nullptr);
    FFTInplaceArray clone(double drift = 1.0);

    FFTInplaceArray& transform (void);
    FFTInplaceArray& transform_back (void);
    FFTInplaceArray correlate_with (FFTInplaceArray &with);
    FFTInplaceArray& assign_correlate_with (FFTInplaceArray &assign_to, FFTInplaceArray &with);
    std::shared_ptr<PlanAndBuffer> m_FFT;

    std::complex<double>* const& envelope (void) const { return m_FFT->m_Buffer; }
    std::complex<double>& envelope (size_t i) const { i=(i < m_ActualComplexSize)?i:(m_ActualComplexSize-1); return m_FFT->m_Buffer[i]; }

  protected:
    size_t m_MinimumSize;
    size_t m_ActualComplexSize;
    bool m_IsTransformed = false;

    static size_t compute_actual_complex_size (size_t minimum_size);
};

#endif

