rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );


RT_PROGRAM void cloud_intersect( int primIdx ) {
  int3 v_idx = vindex_buffer[primIdx];

  float3 p0 = vertex_buffer[ v_idx.x ];
  float3 p1 = vertex_buffer[ v_idx.y ];
  float3 p2 = vertex_buffer[ v_idx.z ];

  // Intersect ray with triangle
  float3 n;
  float  t, beta, gamma;
  if( intersect_triangle( ray, p0, p1, p2, n, t, beta, gamma ) ) {

    if(  rtPotentialIntersection( t ) ) {

      // Calculate normals and tex coords
      float3 geo_n = normalize( n );
      int3 n_idx = nindex_buffer[ primIdx ];
      if ( normal_buffer.size() == 0 || n_idx.x < 0 || n_idx.y < 0 || n_idx.z < 0 ) {
        shading_normal = geo_n;
      } else {
        float3 n0 = normal_buffer[ n_idx.x ];
        float3 n1 = normal_buffer[ n_idx.y ];
        float3 n2 = normal_buffer[ n_idx.z ];
        shading_normal = normalize( n1*beta + n2*gamma + n0*(1.0f-beta-gamma) );
      }
      geometric_normal = geo_n;

      int3 t_idx = tindex_buffer[ primIdx ];
      if ( texcoord_buffer.size() == 0 || t_idx.x < 0 || t_idx.y < 0 || t_idx.z < 0 ) {
        texcoord = make_float3( 0.0f, 0.0f, 0.0f );
      } else {

        float2 t0 = texcoord_buffer[ t_idx.x ];
        float2 t1 = texcoord_buffer[ t_idx.y ];
        float2 t2 = texcoord_buffer[ t_idx.z ];
        texcoord = make_float3( t1*beta + t2*gamma + t0*(1.0f-beta-gamma) );
      }

      refine_and_offset_hitpoint( ray.origin + t*ray.direction, ray.direction,
                                  geo_n, p0,
                                  back_hit_point, front_hit_point );

      rtReportIntersection( material_buffer[primIdx] );
    }
  }
}
