#include <math.h>
#include <float.h>
#include <stdio.h>

#include "../struct_kernel.h"
#include "regularized-coulomb.h"

void regularizedCoulombDirect(int number_of_targets_in_batch, int number_of_source_points_in_cluster,
        int starting_index_of_target, int starting_index_of_source,
        double *target_x, double *target_y, double *target_z,
        double *source_x, double *source_y, double *source_z, double *source_charge, double *source_weight,
        struct kernel *kernel, double *potential, int gpu_async_stream_id)
{
    double epsilon=kernel->parameters[0];

#ifdef OPENACC_ENABLED
    #pragma acc kernels async(gpu_async_stream_id) present(target_x, target_y, target_z, \
                        source_x, source_y, source_z, source_charge, source_weight, potential)
    {
    #pragma acc loop independent
#endif
    for (int i = 0; i < number_of_targets_in_batch; i++) {

        int ii = starting_index_of_target + i;
        double temporary_potential = 0.0;

        double tx = target_x[ii];
        double ty = target_y[ii];
        double tz = target_z[ii];

#ifdef OPENACC_ENABLED
        #pragma acc loop independent reduction(+:temporary_potential)
#endif
        for (int j = 0; j < number_of_source_points_in_cluster; j++) {
#ifdef OPENACC_ENABLED
            #pragma acc cache(source_x[starting_index_of_source : starting_index_of_source+number_of_source_points_in_cluster-1], \
                              source_y[starting_index_of_source : starting_index_of_source+number_of_source_points_in_cluster-1], \
                              source_z[starting_index_of_source : starting_index_of_source+number_of_source_points_in_cluster-1], \
                              source_charge[starting_index_of_source : starting_index_of_source+number_of_source_points_in_cluster-1], \
                              source_weight[starting_index_of_source : starting_index_of_source+number_of_source_points_in_cluster-1])
#endif


            int jj = starting_index_of_source + j;
            double dx = tx - source_x[jj];
            double dy = ty - source_y[jj];
            double dz = tz - source_z[jj];
            double r2  = dx*dx + dy*dy + dz*dz + epsilon*epsilon;

            temporary_potential += source_charge[jj] * source_weight[jj] / sqrt(r2);
        } // end loop over interpolation points
#ifdef OPENACC_ENABLED
        #pragma acc atomic
#endif
        potential[ii] += temporary_potential;
    }
#ifdef OPENACC_ENABLED
    } // end kernel
#endif
    return;
}




void regularizedCoulombApproximationLagrange(int number_of_targets_in_batch, int number_of_interpolation_points_in_cluster,
         int starting_index_of_target, int starting_index_of_cluster,
         double *target_x, double *target_y, double *target_z,
         double *cluster_x, double *cluster_y, double *cluster_z, double *cluster_charge,
         struct kernel *kernel, double *potential, int gpu_async_stream_id)
{

    double epsilon=kernel->parameters[0];

#ifdef OPENACC_ENABLED
    #pragma acc kernels async(gpu_async_stream_id) present(target_x, target_y, target_z, \
                        cluster_x, cluster_y, cluster_z, cluster_charge, potential)
    {
#endif
#ifdef OPENACC_ENABLED
    #pragma acc loop independent
#endif	
    for (int i = 0; i < number_of_targets_in_batch; i++) {

        double temporary_potential = 0.0;

        double tx = target_x[starting_index_of_target + i];
        double ty = target_y[starting_index_of_target + i];
        double tz = target_z[starting_index_of_target + i];

#ifdef OPENACC_ENABLED
        #pragma acc loop independent reduction(+:temporary_potential)
#endif
        for (int j = 0; j < number_of_interpolation_points_in_cluster; j++) {
#ifdef OPENACC_ENABLED
            #pragma acc cache(cluster_x[starting_index_of_cluster : starting_index_of_cluster+number_of_interpolation_points_in_cluster-1], \
                              cluster_y[starting_index_of_cluster : starting_index_of_cluster+number_of_interpolation_points_in_cluster-1], \
                              cluster_z[starting_index_of_cluster : starting_index_of_cluster+number_of_interpolation_points_in_cluster-1], \
                              cluster_charge[starting_index_of_cluster : starting_index_of_cluster+number_of_interpolation_points_in_cluster-1])
#endif

            int jj = starting_index_of_cluster + j;
            double dx = tx - cluster_x[jj];
            double dy = ty - cluster_y[jj];
            double dz = tz - cluster_z[jj];
            double r2  = dx*dx + dy*dy + dz*dz + epsilon*epsilon;

            if (r2 > DBL_MIN) {
                temporary_potential += cluster_charge[starting_index_of_cluster + j] / sqrt(r2);
            }
        } // end loop over interpolation points
#ifdef OPENACC_ENABLED
        #pragma acc atomic
#endif
        potential[starting_index_of_target + i] += temporary_potential;
    }
#ifdef OPENACC_ENABLED
    } // end kernel
#endif
    return;
}




void regularizedCoulombApproximationHermite(int number_of_targets_in_batch, int number_of_interpolation_points_in_cluster,
        int starting_index_of_target, int starting_index_of_cluster, int total_number_interpolation_points,
        double *target_x, double *target_y, double *target_z,
        double *cluster_x, double *cluster_y, double *cluster_z, double *cluster_charge,
        struct kernel *kernel, double *potential, int gpu_async_stream_id)
{

    double epsilon=kernel->parameters[0];

    printf("\n\nWARNING: Hermite regularized coulomb has not been implemented yet.\n\n\n");

    // total_number_interpolation_points is the stride, separating clustersQ, clustersQx, clustersQy, etc.
    double *cluster_charge_          = &cluster_charge[8*starting_index_of_cluster + 0*number_of_interpolation_points_in_cluster];
    double *cluster_charge_delta_x   = &cluster_charge[8*starting_index_of_cluster + 1*number_of_interpolation_points_in_cluster];
    double *cluster_charge_delta_y   = &cluster_charge[8*starting_index_of_cluster + 2*number_of_interpolation_points_in_cluster];
    double *cluster_charge_delta_z   = &cluster_charge[8*starting_index_of_cluster + 3*number_of_interpolation_points_in_cluster];
    double *cluster_charge_delta_xy  = &cluster_charge[8*starting_index_of_cluster + 4*number_of_interpolation_points_in_cluster];
    double *cluster_charge_delta_yz  = &cluster_charge[8*starting_index_of_cluster + 5*number_of_interpolation_points_in_cluster];
    double *cluster_charge_delta_xz  = &cluster_charge[8*starting_index_of_cluster + 6*number_of_interpolation_points_in_cluster];
    double *cluster_charge_delta_xyz = &cluster_charge[8*starting_index_of_cluster + 7*number_of_interpolation_points_in_cluster];



#ifdef OPENACC_ENABLED
    #pragma acc kernels async(gpu_async_stream_id) present(target_x, target_y, target_z, \
                        cluster_x, cluster_y, cluster_z, cluster_charge, potential, \
                        cluster_charge_, cluster_charge_delta_x, cluster_charge_delta_y, cluster_charge_delta_z, \
                        cluster_charge_delta_xy, cluster_charge_delta_yz, cluster_charge_delta_xz, \
                        cluster_charge_delta_xyz)
    {
#endif
#ifdef OPENACC_ENABLED
    #pragma acc loop independent
#endif
    for (int i = 0; i < number_of_targets_in_batch; i++) {

        int ii = starting_index_of_target + i;
        double temporary_potential = 0.0;

        double tx = target_x[ii];
        double ty = target_y[ii];
        double tz = target_z[ii];

#ifdef OPENACC_ENABLED
        #pragma acc loop independent reduction(+:temporary_potential)
#endif
        for (int j = 0; j < number_of_interpolation_points_in_cluster; j++) {

            int jj = starting_index_of_cluster + j;
            double dx = tx - cluster_x[jj];
            double dy = ty - cluster_y[jj];
            double dz = tz - cluster_z[jj];
            double r  = sqrt(dx*dx + dy*dy + dz*dz);

            double rinv  = 1 / r;
            double r3inv = rinv*rinv*rinv;
            double r5inv = r3inv*rinv*rinv;
            double r7inv = r5inv*rinv*rinv;

            if (r > DBL_MIN) {

                temporary_potential +=  rinv  * (cluster_charge_[j])
                                 +      r3inv * (cluster_charge_delta_x[j]*dx + cluster_charge_delta_y[j]*dy
                                               + cluster_charge_delta_z[j]*dz)
                                 +  3 * r5inv * (cluster_charge_delta_xy[j]*dx*dy + cluster_charge_delta_yz[j]*dy*dz
                                               + cluster_charge_delta_xz[j]*dx*dz)
                                 + 15 * r7inv *  cluster_charge_delta_xyz[j]*dx*dy*dz;

            }
        } // end loop over interpolation points
#ifdef OPENACC_ENABLED
        #pragma acc atomic
#endif
        potential[starting_index_of_target + i] += temporary_potential;
    }
#ifdef OPENACC_ENABLED
    } // end kernel
#endif
    return;
}