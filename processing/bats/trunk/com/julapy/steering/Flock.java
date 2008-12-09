package com.julapy.steering;

import toxi.geom.Vec3D;

public class Flock
{
	public float centerPullScale			= 0.0001f;
	public float collisionAvoidanceScale	= 0.01f;
	public float flockAverageScale	 		= 0.0008f;
	public float targetPullScale			= 0.0005f;
	
	public float minDistance	= 50;
	public float velocityLimit	= 30;
	public float flockRange		= 200;
	public float targetRange	= 100;

//	public float centerPullScale			= 0.01f;
//	public float collisionAvoidanceScale	= 0.01f;
//	public float flockAverageScale	 		= 0.0008f;
//	public float targetPullScale			= 0.05f;
//	
//	public float minDistance	= 50;
//	public float velocityLimit	= 10;
//	public float flockRange		= 100;
	
	private Particle[] items;
	public	Vec3D flockTarget;

	public Flock ( Particle[] items )
	{
		this.items = items;
	}
	
	public Flock ( Particle[] items, Vec3D flockTarget )
	{
		this.items			= items;
		this.flockTarget	= flockTarget;
	}
	
	public void update ( )
	{
		Particle item;
		int i;
		
		for( i=0; i<items.length; i++ )
		{
			item = items[ i ];
			
			Vec3D v1 = rule1( item );	// pull to flock center
			Vec3D v2 = rule2( item );	// avoid collision.
			Vec3D v3 = rule3( item );	// head to flock average
			Vec3D v4 = rule4( item );	// pull to flock target
			
			v1.scaleSelf( centerPullScale );
			v2.scaleSelf( collisionAvoidanceScale );
			v3.scaleSelf( flockAverageScale );
			v4.scaleSelf( getLinearScale( item.loc.distanceTo( flockTarget ), targetRange, targetPullScale ) );
			
			item.vel.addSelf( v1 );
			item.vel.addSelf( v2 );
			item.vel.addSelf( v3 );
			item.vel.addSelf( v4 );
			
			item.vel.scaleSelf( getLimit( item.vel.magnitude(), velocityLimit ) );
//			item.vel.scaleSelf( getLinearScale( item.loc.distanceTo( flockTarget ), targetRange, 0.001f ) + 1 );
		}
	}
	
	////////////////////////////////////////////////////
	// pull to the center
	////////////////////////////////////////////////////
	
	public Vec3D rule1 ( Particle item1 )
	{
		Particle item2;
		Vec3D v;
		float d;
		int count, i;
		
		v 		= new Vec3D();
		count	= 0;
		d		= 0;
		
		for( i=0; i<items.length; i++ )
		{
			item2 = items[ i ];

			if( item1 != item2 )
			{
				d = item1.loc.distanceTo( item2.loc );
				
				if( d < flockRange )
				{
					v.addSelf( item2.loc );
					
					++count;
				}
			}
		}
		
		if(count > 0)
		{
			v.x /= count;
			v.y /= count;
			v.z /= count;
			
			v.x = ( v.x - item1.loc.x);
			v.y = ( v.y - item1.loc.y);
			v.z = ( v.z - item1.loc.z);
		}
		
		return v;
	}
	
	////////////////////////////////////////////////////
	// avoid collision with other particles.
	////////////////////////////////////////////////////
	
	public Vec3D rule2( Particle item1 )
	{
		Particle item2;
		Vec3D v;
		int i;
		
		v = new Vec3D( );
		
		for( i=0; i<items.length; i++)
		{
			item2 = items[ i ];
			
			if ( item1 != item2 )
			{
				if ( item1.loc.distanceTo( item2.loc ) < minDistance )
				{
					v.x -= item2.loc.x - item1.loc.x;
					v.y -= item2.loc.y - item1.loc.y;
					v.z -= item2.loc.z - item1.loc.z;
				}
			}
		}

		return v;
	}
	
	////////////////////////////////////////////////////	
	// head to flock average
	////////////////////////////////////////////////////	
	
	public Vec3D rule3( Particle item1 )
	{
		Particle item2;
		Vec3D v;
		float d;
		int count, i;
		
		v		= new Vec3D();
		count	= 0;
		
		for( i=0; i<items.length; i++ )
		{
			item2 = items[ i ];
			
			if ( item1 != item2 )
			{
				d = item1.loc.distanceTo( item2.loc );
				
				if( d < flockRange )
				{
					v.x += item2.vel.x;
					v.y += item2.vel.y;
					v.z += item2.vel.z;
					
					++count;
				}
			}
		}
		
		v.x /= count;
		v.y /= count;
		v.z /= count;
		
		v.x = ( v.x - item1.vel.x );
		v.y = ( v.y - item1.vel.y );
		v.z = ( v.z - item1.vel.z );
		
		return v;
	}
	
	////////////////////////////////////////////////////	
	// pull to target
	////////////////////////////////////////////////////	

	public Vec3D rule4( Particle item1 )
	{
		Vec3D v;
		
		if( flockTarget != null )
		{
			v = new Vec3D();

			v.x	= ( flockTarget.x - item1.loc.x);
			v.y	= ( flockTarget.y - item1.loc.y);
			v.z	= ( flockTarget.z - item1.loc.z);
			
			return v;
		}

		return new Vec3D();
	}

	public Vec3D limitVelocity (Vec3D v)
	{
		float vel = v.magnitude();

	    if (vel > velocityLimit)
	    {
	    	v.scaleSelf( velocityLimit / vel );
	    }
	    
	    return v;
	}
	
	private float getLimit( float v1, float v2 )
	{
		if( v1 > v2 )
		{
			return v2 / v1;
		}
		else
		{
			return 1;
		}
	}
	
	private float getLinearScale ( float distance, float range, float rise )
	{
		if( distance > range )
			return ( distance - range ) * rise;
		else
			return 0;
	}
}